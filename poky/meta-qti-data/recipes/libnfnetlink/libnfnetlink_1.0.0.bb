DESCRIPTION = "libnfnetlink is the low-level library for netfilter \
related kernel/userspace communication"
SECTION = "devel/libs"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://README;md5=287b7901363dda6ef7176fadee6972ed"

PR = "r0"

SRC_URI = "http://www.netfilter.org/projects/libnfnetlink/files/libnfnetlink-${PV}.tar.bz2"

inherit autotools pkgconfig

SRC_URI[md5sum] = "016fdec8389242615024c529acc1adb8"
SRC_URI[sha256sum] = "3752b03a4c09821ee9a2528d69289423a01e7171f1a22dfdd11d5459e03972fb"
