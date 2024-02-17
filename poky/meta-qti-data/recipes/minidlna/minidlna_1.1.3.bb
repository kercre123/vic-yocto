inherit autotools-brokensep gettext
SUMMARY = "MiniDLNA (aka ReadyDLNA) is server software with the aim of being fully \
compliant with DLNA/UPnP-AV clients."
HOMEPAGE = "http://sourceforge.net/projects/minidlna/"
BUGTRACKER = "http://sourceforge.net/tracker/?group_id=243163&atid=1121516&source=navbar"
LICENSE = "GPLv2"
PRIORITY = "optional"
DEPENDS = "libvorbis sqlite3 libexif libjpeg-turbo libid3tag ffmpeg flac zlib"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

# Package Revision (update whenever recipe is changed)
PR = "r5"

SRC_URI = "\
    http://sourceforge.net/projects/minidlna/files/minidlna/${PV}/${PN}-${PV}.tar.gz \
    file://0001-certification-fixes.patch \
    file://0001-M-DMS.patch \
    file://0001-initial-notify.patch \
    file://0001-chunked-data.patch \
    file://0001-search-criteria.patch \
    file://init.patch \
"

SRC_URI[md5sum] = "879027192c89e5376cdd2ae2d1aa33b4"
SRC_URI[sha256sum] = "ed42d5cadf9488a95a0107341918879ef8ce4c650e19337688c46cdcd484bc4e"

do_install_append () {
    sed -i s:#network_interface=eth0:network_interface=bridge0,ppp0:g minidlna.conf
    sed -i s:"#friendly_name=My DLNA Server":"friendly_name=9x35 MobileAP DLNA":g minidlna.conf
    install -d ${D}${sysconfdir}
    install -d ${D}${sysconfdir}/data/
    install -m 644  ${WORKDIR}/${PN}-${PV}/${PN}.conf ${D}${sysconfdir}/data/
    install -d ${D}${sysconfdir}/init.d/
    install ${WORKDIR}/${PN}-${PV}/linux/${PN}.init.d.script ${D}${sysconfdir}/init.d/minidlna
}
FILES_${PN} += "${sysconfdir}/data/minidlna.conf"
