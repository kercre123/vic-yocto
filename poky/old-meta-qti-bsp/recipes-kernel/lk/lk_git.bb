inherit deploy qperf

DESCRIPTION = "Little Kernel bootloader"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=0835ade698e0bcf8506ecda2f7b4f302"
HOMEPAGE = "https://www.codeaurora.org/gitweb/quic/la?p=kernel/lk.git"

PROVIDES = "virtual/bootloader"

PR       = "r1"

PACKAGE_ARCH = "${MACHINE_ARCH}"
FILESPATH =+ "${WORKSPACE}:"

SRC_URI   =  "file://bootable/bootloader/lk"
S         =  "${WORKDIR}/bootable/lk"

MY_TARGET_mdm9650 = "mdm9640"
MY_TARGET_sdx20 = "mdm9640"
MY_TARGET_apq8009  = "msm8909"
MY_TARGET_msm8909  = "msm8909"
MY_TARGET_msm8909w = "msm8909"
MY_TARGET_apq8096  = "msm8996"
MY_TARGET_mdm9607  = "mdm9607"
MY_TARGET_apq8053  = "msm8953"
MY_TARGET_apq8017  = "msm8952"
MY_TARGET         ?= "${BASEMACHINE}"

BOOTLOADER_NAME = "${@base_contains('DISTRO_FEATURES', 'emmc-boot', 'emmc_appsboot', 'appsboot', d)}"

emmc_bootloader = "${@base_contains('DISTRO_FEATURES', 'emmc-boot', '1', '0', d)}"

LIBGCC = "${@base_contains('GCCVERSION', '5.2%', '${STAGING_LIBDIR}/${TARGET_SYS}/5.2.0/libgcc.a', '${STAGING_LIBDIR}/${TARGET_SYS}/4.9.3/libgcc.a', d)}"

# Disable display completely for Victor
DISPLAY_SCREEN = "0"

ENABLE_DISPLAY = "${DISPLAY_SCREEN}"

EXTRA_OEMAKE = "${MY_TARGET} TOOLCHAIN_PREFIX='${TARGET_PREFIX}'  LIBGCC='${LIBGCC}' DISPLAY_SCREEN=${DISPLAY_SCREEN} ENABLE_DISPLAY=${ENABLE_DISPLAY}"

EXTRA_OEMAKE_append_mdm9650 = " ENABLE_EARLY_ETHERNET=1"

EXTRA_OEMAKE_append = " VERIFIED_BOOT=0 DEFAULT_UNLOCK=true EMMC_BOOT=${emmc_bootloader} APPEND_CMDLINE=${emmc_bootloader}"

EXTRA_OEMAKE_append = " ${@base_contains('DISTRO_FEATURES', 'systemd', 'USE_LE_SYSTEMD=true', '', d)}"

EXTRA_OEMAKE_append = " ${@base_contains('DISTRO_FEATURES', 'vble', 'VERIFIED_BOOT_LE=1', '', d)}"

EXTRA_OEMAKE_append = " ${@base_conditional('USER_BUILD', '1', 'TARGET_BUILD_VARIANT=user', '', d)}"

python () {
    qsn = os.environ.get('QSN', False)
    bb.note("LK QSN_LOCK={}".format(qsn))
    if qsn:
        d.appendVar('EXTRA_OEMAKE', ' QSN_LOCK=' + qsn)
}

#enable hardfloat
EXTRA_OEMAKE_append = " ${@base_conditional('ARM_FLOAT_ABI', 'hard', 'ENABLE_HARD_FPU=1', '', d)}"

do_install() {
        install -d ${D}/boot
        install build-${MY_TARGET}/*.mbn ${D}/boot
}


FILES_${PN} = "/boot"
FILES_${PN}-dbg = "/boot/.debug"

do_deploy() {
        mkdir -p ${DEPLOY_DIR_IMAGE}
        install ${S}/build-${MY_TARGET}/*.mbn ${DEPLOY_DIR_IMAGE}
}

do_deploy[dirs] = "${S} ${DEPLOYDIR}"
addtask deploy before do_build after do_install

PACKAGE_STRIP = "no"
