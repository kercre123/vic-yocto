DESCRIPTION = "A sophisticated network protocol analyzer"
HOMEPAGE = "http://www.tcpdump.org/"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=1d4b0366557951c84a94fabe3529f867"
SECTION = "console/network"
PRIORITY = "optional"
DEPENDS = "libpcap libtirpc"
PR = "r2"
export LIBS=" -lpcap"

SRC_URI = " \
	http://www.tcpdump.org/release/tcpdump-${PV}.tar.gz;name=tarball \
	file://tcpdump_configure_no_-O2.patch \
	file://0001-minimal-IEEE802.15.4-allowed.patch \
	file://ipv6-cross.patch \
	file://configure.patch \
        https://raw.githubusercontent.com/openembedded/meta-oe/master/meta-networking/recipes-support/tcpdump/tcpdump/unnecessary-to-check-libpcap.patch;name=patch \
"

inherit autotools-brokensep
CACHED_CONFIGUREVARS = "ac_cv_linux_vers=${ac_cv_linux_vers=2} td_cv_buggygetaddrinfo=cross"

PACKAGECONFIG ??= "ipv6"
PACKAGECONFIG[openssl] = "--with-crypto=yes, --without-crypto, openssl"
PACKAGECONFIG[ipv6] = "--enable-ipv6, --disable-ipv6,"

CFLAGS_append = " -I/usr/include/tirpc "
LDFLAGS_append = " -ltirpc "

do_configure_prepend () {
        #Allow build paths with containing AU_
        sed 's|AC_CANONICAL_HOST|m4_pattern_allow([^AU_])\nAC_CANONICAL_HOST|' -i ${S}/configure.in
}

do_configure() {
	sed -i 's:-L/lib:-L${STAGING_LIBDIR}:g' ./configure.in

	gnu-configize
	autoconf  -v -f
	oe_runconf
	sed -i 's:/usr/lib:${STAGING_LIBDIR}:' ./Makefile
	sed -i 's:/usr/include:${STAGING_INCDIR}:' ./Makefile
}

do_install_append() {
	# tcpdump 4.0.0 installs a copy to /usr/sbin/tcpdump.4.0.0
	rm -f ${D}${sbindir}/tcpdump.${PV}
}

SRC_URI[tarball.md5sum] = "d0dd58bbd6cd36795e05c6f1f74420b0"
SRC_URI[tarball.sha256sum] = "e6cd4bbd61ec7adbb61ba8352c4b4734f67b8caaa845d88cb826bc0b9f1e7f0a"
SRC_URI[patch.md5sum] = "a59ceb1b4cbda7a94f682ee51990d2f7"
SRC_URI[patch.sha256sum] = "c8273bdf22860e2c00fd04e168f8cd44385d83591a6529f6e888a845b804e6a9"
