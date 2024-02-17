DESCRIPTION = "Start up script for updating a few persist and data file permissions post OTA"
HOMEPAGE = "http://us.codeaurora.org"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=550794465ba0ec5312d6919e203a55f9"

SRC_URI +="file://${BASEMACHINE}/update_file_permissions.sh"

S = "${WORKDIR}/${BASEMACHINE}"

PR = "r4"

inherit update-rc.d

INITSCRIPT_NAME = "update_file_permissions.sh"
INITSCRIPT_PARAMS = "start 37 S ."

do_install() {
    install -m 0500 ${WORKDIR}/${BASEMACHINE}/update_file_permissions.sh -D ${D}${sysconfdir}/init.d/update_file_permissions.sh
}
