include selinux_20180524.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343"

SRC_URI[md5sum] = "62ed7bb2ede677a735f2750751677a4f"
SRC_URI[sha256sum] = "1c0de8d2c51e5460926c21e371105c84a39087dfd8f8e9f0cc1d017e4cbea8e2"

SRC_URI += "\
	file://libsemanage-Fix-execve-segfaults-on-Ubuntu.patch \
	file://libsemanage-fix-path-nologin.patch \
	file://libsemanage-drop-Wno-unused-but-set-variable.patch \
	file://libsemanage-define-FD_CLOEXEC-as-necessary.patch;striplevel=2 \
	file://libsemanage-allow-to-disable-audit-support.patch \
	file://libsemanage-disable-expand-check-on-policy-load.patch \
	file://0001-src-Makefile-fix-includedir-in-libselinux.pc.patch \
	"
FILES_${PN} += "/usr/libexec"
