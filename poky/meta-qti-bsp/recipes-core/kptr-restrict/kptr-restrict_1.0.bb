inherit update-rc.d

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

DESCRIPTION = "Script to restrict kernel pointer access"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=550794465ba0ec5312d6919e203a55f9"

SRC_URI += "file://logging-restrictions.sh"

PR = "r0"

do_configure[noexec] = "1"
do_compile[noexec]   = "1"

INITSCRIPT_NAME = "logging-restrictions.sh"
INITSCRIPT_PARAMS = "start 39 S ."

do_install() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
        install -d ${D}${sysconfdir}/kptr-restrict
        install -d ${D}${sysconfdir}/kptr-restrict/init.d
        install -m 0755 ${WORKDIR}/logging-restrictions.sh  ${D}${sysconfdir}/init.d/logging-restrictions.sh
        update-rc.d -r ${D} logging-restrictions.sh start 39 S .
    fi
}
