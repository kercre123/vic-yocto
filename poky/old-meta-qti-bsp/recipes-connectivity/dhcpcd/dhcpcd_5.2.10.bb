SECTION = "console/network"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"

DESCRIPTION = "dhcpcd is an RFC2131-, RFC2132-, and \
RFC1541-compliant DHCP client daemon. It gets an IP address \
and other information from the DHCP server, automatically \
configures the network interface, and tries to renew the \
lease time according to RFC2131 or RFC1541."

PR = "r2"

inherit autotools-brokensep

#SRC_URI = "http://roy.aydogan.net/dhcpcd/dhcpcd-${PV}.tar.bz2"
SRC_URI = "https://web.archive.org/web/20150423173746if_/http://roy.aydogan.net/dhcpcd/dhcpcd-5.2.10.tar.bz2"

do_configure() {
        ./configure --includedir=${STAGING_INCDIR} --bindir=${prefix}/sbin \
        --sbindir=${exec_prefix}/sbin
}

FILES_${PN} +="/usr/libexec/*"
FILES_${PN} += "/usr/etc*"
FILES_${PN} += "/data/*"

SRC_URI[md5sum] = "c65e8cef3281eaf2e12a84bd882f5c63"
SRC_URI[sha256sum] = "d3325c697d6e2a2d09c80cedb358bb78561da33304183874d7c44f96fd5d9f5f"
