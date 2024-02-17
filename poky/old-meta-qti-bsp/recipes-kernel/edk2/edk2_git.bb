inherit deploy
DESCRIPTION = "UEFI bootloader"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=0835ade698e0bcf8506ecda2f7b4f302"

PROVIDES = "virtual/bootloader"
PV       = "3.0"
PR       = "r1"

LLVM_PREBUILTS_BASE = "prebuilts/clang/host"
LLVM_PREBUILTS_VERSION = "clang-2690385"
BUILD_OS = "linux"

PACKAGE_ARCH = "${MACHINE_ARCH}"
FILESPATH =+ "${WORKSPACE}:"

SRC_URI = "file://bootable/bootloader/edk2"
S         =  "${WORKDIR}/bootable/edk2"

INSANE_SKIP_${PN} = "arch"

EXTRA_OEMAKE = "'ANDROID_BUILD_TOP=${WORKSPACE}'\
                'TARGET_GCC_VERSION=4.9'\
                'LLVM_PREBUILTS_PATH=${LLVM_PREBUILTS_BASE}/${BUILD_OS}-x86/${LLVM_PREBUILTS_VERSION}/bin'\
                'CLANG_BIN=${WORKSPACE}/${LLVM_PREBUILTS_PATH}/'\
                'BUILDDIR=${S}'\
                'BOOTLOADER_OUT=${S}/out'\
                'EDK_TOOLS_PATH=${S}/BaseTools'"

do_compile () {
    unset CC
    unset ARCH
    unset CXX
    unset LD
    unset LINKER
    oe_runmake -f makefile all
}
do_install() {
        install -d ${D}/boot
}

FILES_${PN} = "/boot"
FILES_${PN}-dbg = "/boot/.debug"

do_deploy() {
        install ${D}/boot/abl.elf ${DEPLOYDIR}
}

do_deploy[dirs] = "${S} ${DEPLOYDIR}"
addtask deploy before do_build after do_install

PACKAGE_STRIP = "no"