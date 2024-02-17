inherit update-rc.d autotools-brokensep

DESCRIPTION = "Daemon to start QRNG"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
BSD;md5=3775480a712fc46a69647678acb234cb"
PR = "r3"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://external/qrngd/"
S = "${WORKDIR}/external/qrngd/"

INITSCRIPT_NAME = "qrngd"
INITSCRIPT_PARAMS = "start 97 2 3 4 5 . stop 03 0 1 6 ."

do_install_append() {
        install -m 0755 ${S}/start_qrngd -D ${D}${sysconfdir}/init.d/qrngd
        install -m 0755 ${S}/qrngd -D ${D}/bin/qrngd
        install -m 0755 ${S}/qrngtest -D ${D}/bin/qrngtest
}
