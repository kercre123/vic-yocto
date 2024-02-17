inherit autotools-brokensep linux-kernel-base pkgconfig

DESCRIPTION = "Hostap Daemon"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILES_${PN} += "\
        /usr/bin \
        "
INC_PR = "r4"

DEPENDS = "openssl libnl"

SRC_URI = "file://${WORKSPACE}/external/hostap/"

S = "${WORKDIR}/external/hostap/hostapd/"

do_install() {
    make install DESTDIR=${D} BINDIR=${sbindir}/
}
