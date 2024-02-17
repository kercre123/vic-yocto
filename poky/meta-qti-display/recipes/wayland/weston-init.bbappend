SUMMARY = "Startup script for the Weston Wayland compositor"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690"
S = "${WORKDIR}/weston-init"

FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI_append = "\
    file://init_qti \
"

do_install() {
    install -d ${D}/${sysconfdir}/init.d
    install -m755 ${WORKDIR}/init_qti ${D}/${sysconfdir}/init.d/weston
}
