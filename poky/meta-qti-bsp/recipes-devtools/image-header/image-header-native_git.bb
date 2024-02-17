inherit native

DESCRIPTION = "ELF wrapper script from edk"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/${LICENSE};md5=550794465ba0ec5312d6919e203a55f9"

PROVIDES = "image-header-native"

FILESPATH =+ "${WORKSPACE}/bootable/bootloader/edk2/:"
SRC_URI   = "file://QcomModulePkg/Tools/"

PR = "r1"

do_compile[noexec] = "1"
do_configure[noexec] = "1"

do_install() {
	install -d ${D}${bindir}
	install ${WORKDIR}/QcomModulePkg/Tools/image_header.py ${D}${bindir}
	install ${WORKDIR}/QcomModulePkg/Tools/elf_tools.py ${D}${bindir}
}
