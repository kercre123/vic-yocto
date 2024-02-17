SUMMARY = "Point-to-Point Protocol (PPP) support"
DESCRIPTION = "ppp (Paul's PPP Package) is an open source package which implements \
the Point-to-Point Protocol (PPP) on Linux and Solaris systems."

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://ipv6-up \
           file://disconnect"

do_install_append () {
	install -m 0755 ${WORKDIR}/ipv6-up ${D}${sysconfdir}/ppp/
	install -m 0755 ${WORKDIR}/disconnect ${D}${sysconfdir}/
}
