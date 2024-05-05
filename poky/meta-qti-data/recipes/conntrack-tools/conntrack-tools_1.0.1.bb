SUMMARY = "Tools for managing kernel packet filtering capabilities"
DESCRIPTION = "The conntrack-tools are a set of free software tools for \
GNU/Linux that allow system administrators interact, from user-space, \
with the in-kernel Connection Tracking System."
HOMEPAGE = "http://www.netfilter.org/"
BUGTRACKER = "http://bugzilla.netfilter.org/"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=8ca43cbc842c2336e835926c2166c28b"
DEPENDS = "bison-native libnfnetlink libnetfilter-conntrack"

PR = "r1"

SRC_URI = " \
	http://netfilter.org/projects/conntrack-tools/files/conntrack-tools-${PV}.tar.bz2 \
        file://conntrack-tools-NULL.patch \
        file://conntrack-tools-ipv6.patch \
"

SRC_URI[md5sum] = "8a60f02a177fc31fe40cc992c4de90e2"
SRC_URI[sha256sum] = "1e5769a17ed17e1e8886a1807af00acda4cceec996d194f0519d922e41655380"

inherit autotools pkgconfig
