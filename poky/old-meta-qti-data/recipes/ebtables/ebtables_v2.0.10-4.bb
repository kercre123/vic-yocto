inherit autotools-brokensep

DESCRIPTION = "Ethernet bridge tables - Linux Ethernet filter for the Linux bridge"
HOMEPAGE = "http://ebtables.sourceforge.net/"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=53b4a999993871a28ab1488fdbd2e73e"
RDEPENDS_${PN}  += "perl"
SECTION  = "console/network"
PRIORITY = "optional"
DEPENDS        = "virtual/kernel"
PR = "r2"

TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI = " \
        http://sourceforge.net/projects/ebtables/files/ebtables/ebtables-2-0-10-4/ebtables-${PV}.tar.gz \
        file://Makefile.patch \
"
SRC_URI[md5sum] = "506742a3d44b9925955425a659c1a8d0"
SRC_URI[sha256sum] = "dc6f7b484f207dc712bfca81645f45120cb6aee3380e77a1771e9c34a9a4455d"

CACHED_CONFIGUREVARS = "ac_cv_linux_vers=${ac_cv_linux_vers=2}"

EXTRA_OEMAKE = "KERNEL_INCLUDES=${STAGING_KERNEL_BUILDDIR}/usr/include"

EXTRA_OECONF = "--without-crypto \
        ${@bb.utils.contains('DISTRO_FEATURES', 'ipv6', '--enable-ipv6', '--disable-ipv6', d)}"

PACKAGES = "${PN} ${PN}-doc"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INSANE_SKIP_${PN} += "file-rdeps"

FILES_${PN}     += "${libdir}/lib*.so"
FILES_${PN}     += "${sbindir}/*"
FILES_${PN}-doc += "${mandir}/*"

do_configure() {
        :
}
do_configure_append() {
    if [ -d "${S}" ]; then
        install -m 555 ${S}/include/linux/netfilter_bridge/ebt_ulog.h ${STAGING_KERNEL_BUILDDIR}/usr/include/linux/netfilter_bridge/ebt_ulog.h
    fi
}
