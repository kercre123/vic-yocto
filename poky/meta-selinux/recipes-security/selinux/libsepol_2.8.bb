include selinux_20180524.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343"

SRC_URI[md5sum] = "c19aa9dde1e78d1c2bd3109579e4d484"
SRC_URI[sha256sum] = "3ad6916a8352bef0bad49acc8037a5f5b48c56f94e4cb4e1959ca475fa9d24d6"

SRC_URI += "file://0001-src-Makefile-fix-includedir-in-libsepol.pc.patch"
