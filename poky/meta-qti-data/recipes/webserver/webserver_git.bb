inherit autotools update-rc.d

DESCRIPTION = "Reference Webserver"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PR = "r3"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://webserver"

S = "${WORKDIR}/webserver"

EXTRA_OEMAKE_append = " CROSS=${HOST_PREFIX}"

INITSCRIPT_NAME = "webserver"
INITSCRIPT_PARAMS = "start 98 5 . stop 15 0 1 6 ."

do_install() {
    install -m 0755 ${S}/webserver -D ${D}/bin/webserver
    install -m 0755 ${S}/start_webserver -D ${D}${sysconfdir}/init.d/webserver
    install -m 0644 ${S}/default_config.conf -D ${D}${sysconfdir}/webserver.conf
}
