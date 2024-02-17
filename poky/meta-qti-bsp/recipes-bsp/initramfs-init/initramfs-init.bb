DESCRIPTION = "Initialize scripts for initramfs"

LICENSE = "BSD-3-Clause-Clear"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-qti-bsp/files/common-licenses/\
${LICENSE};md5=48b43ba58d0f8e9ef3704313a46b7a43"

RDEPENDS_${PN} += " busybox"
SRC_URI = "file://initramfs_init.sh"
PR = "r0"

do_install() {
    install -m 0755 ${WORKDIR}/initramfs_init.sh ${D}/init

    if ${@bb.utils.contains('DISTRO_FEATURES','nad-fde','true','false',d)}; then
        install -d ${D}${base_libdir}/firmware
        ln -sf /firmware/image ${D}${base_libdir}/firmware/updates
    fi
}

FILES_${PN} += " /init"
FILES_${PN} += " ${@bb.utils.contains('DISTRO_FEATURES', 'nad-fde', '${base_libdir}/firmware/*', '', d)}"
PACKAGE_ARCH = "${MACHINE_ARCH}"
