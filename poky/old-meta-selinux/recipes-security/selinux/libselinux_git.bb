PR = "r99"
PV = "2.2+git${SRCPV}"

include selinux_git.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://LICENSE;md5=84b4d2c6ef954a2d4081e775a270d0d0"

SRC_URI += "\
	file://libselinux-drop-Wno-unused-but-set-variable.patch \
	file://libselinux-make-O_CLOEXEC-optional.patch \
	file://libselinux-make-SOCK_CLOEXEC-optional.patch \
	file://libselinux-define-FD_CLOEXEC-as-necessary.patch \
	file://libselinux-get-pywrap-depends-on-selinux.py.patch \
	"
