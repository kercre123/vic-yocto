inherit deploy
DESCRIPTION = "UEFI bootloader"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPLOY_DIR_IMAGE_NAND ?= "${DEPLOYDIR}"
DEPLOY_DIR_IMAGE_EMMC ?= "${DEPLOYDIR}"
PROVIDES = "virtual/bootloader"
PV       = "3.0"
PR       = "r1"

BUILD_OS = "linux"

PACKAGE_ARCH = "${MACHINE_ARCH}"
FILESPATH =+ "${WORKSPACE}/bootable/bootloader/:"

SRC_URI = "file://edk2"
S         =  "${WORKDIR}/edk2"

INSANE_SKIP_${PN} = "arch"

VBLE = "${@bb.utils.contains('DISTRO_FEATURES', 'vble','1', '0', d)}"

VBLEIMA = "${@bb.utils.contains('DISTRO_FEATURES', 'vbleima','1', '0', d)}"
VBLEEVM = "${@bb.utils.contains('DISTRO_FEATURES', 'vbleevm','1', '0', d)}"

VERITY_ENABLED = "${@bb.utils.contains('DISTRO_FEATURES', 'dm-verity','1', '0', d)}"

EARLY_ETH = "${@bb.utils.contains('DISTRO_FEATURES', 'early-eth', '1', '0', d)}"

EARLY_USB_INIT = "${@bb.utils.contains('DISTRO_FEATURES', 'early-usb-init', '1', '0', d)}"

# There is a requirement to have fixed load address for sa2150p target to support multiple DDR configuration.
FIXED_ABL_LOAD_ADDRESS = "${@bb.utils.contains_any('MACHINE', 'sa2150p sa2150p-nand', '0X8B500000', '', d)}"

EXTRA_OEMAKE = "'CLANG_BIN=${STAGING_BINDIR_NATIVE}/llvm-arm-toolchain/bin/' \
                'CLANG_PREFIX=${STAGING_BINDIR_NATIVE}/${TARGET_SYS}/${TARGET_PREFIX}' \
                'TARGET_ARCHITECTURE=${TARGET_ARCH}'\
                'BUILDDIR=${S}'\
                'BOOTLOADER_OUT=${S}/out'\
                'ENABLE_LE_VARIANT=true'\
                'VERIFIED_BOOT_LE=${VBLE}'\
                'VERITY_LE=${VERITY_ENABLED}'\
                'INTEGRITY_LE_IMA=${VBLEIMA}'\
                'INTEGRITY_LE_EVM=${VBLEEVM}'\
                'INIT_BIN_LE=\"/sbin/init\"'\
                'EDK_TOOLS_PATH=${S}/BaseTools'\
                'EARLY_ETH_ENABLED=${EARLY_ETH}'\
                'OVERRIDE_ABL_LOAD_ADDRESS=${FIXED_ABL_LOAD_ADDRESS}'\
                'TARGET_SUPPORTS_EARLY_USB_INIT=${EARLY_USB_INIT}'"

EXTRA_OEMAKE_append_qcs40x = " 'DISABLE_PARALLEL_DOWNLOAD_FLASH=1'"
NAND_SQUASHFS_SUPPORT = "${@bb.utils.contains('DISTRO_FEATURES', 'nand-squashfs', '1', '0', d)}"
EXTRA_OEMAKE_append = " 'NAND_SQUASHFS_SUPPORT=${NAND_SQUASHFS_SUPPORT}'"
NAND_AB_ATTR_SUPPORT = "${@bb.utils.contains('DISTRO_FEATURES', 'nand-ab', 'true', 'false', d)}"
EXTRA_OEMAKE_append = " 'TARGET_NAND_AB_ATTR_SUPPORT=${NAND_AB_ATTR_SUPPORT}'"
NAD_PARTITION = "${@bb.utils.contains('DISTRO_FEATURES', 'nad-prod', '1', '0', d)}"
EXTRA_OEMAKE_append = " 'NAD_PARTITION=${NAD_PARTITION}'"

INITRAMFS = "${@bb.utils.contains('DISTRO_FEATURES', 'initramfs', '1', '0', d)}"
EXTRA_OEMAKE_append = " 'INITRAMFS_BUNDLE=${INITRAMFS}'"

do_compile () {
    export CC=${BUILD_CC}
    export CXX=${BUILD_CXX}
    export LD=${BUILD_LD}
    export AR=${BUILD_AR}
    oe_runmake -f makefile all
}

do_install() {
        install -d  ${D}/boot
}
do_configure[noexec]="1"

FILES_${PN} = "/boot"
FILES_${PN}-dbg = "/boot/.debug"

do_deploy() {
   if ${@bb.utils.contains('IMAGE_FSTYPES', 'ext4', 'true', 'false', d)}; then
        install -d  ${DEPLOY_DIR_IMAGE_EMMC}
        install ${D}/boot/abl.elf ${DEPLOY_DIR_IMAGE_EMMC}
   fi
   if ${@bb.utils.contains('IMAGE_FSTYPES', 'ubi', 'true', 'false', d)}; then
        install -d  ${DEPLOY_DIR_IMAGE_NAND}
        install ${D}/boot/abl.elf ${DEPLOY_DIR_IMAGE_NAND}
   fi
}

do_deploy[dirs] = "${S} ${DEPLOYDIR}"
addtask deploy before do_build after do_install

PACKAGE_STRIP = "no"
