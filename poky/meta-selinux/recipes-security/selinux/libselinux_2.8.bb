include selinux_20180524.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://LICENSE;md5=84b4d2c6ef954a2d4081e775a270d0d0"

SRC_URI[md5sum] = "56057e60192b21122c1aede8ff723ca2"
SRC_URI[sha256sum] = "31db96ec7643ce10912b3c3f98506a08a9116dcfe151855fd349c3fda96187e1"

SRC_URI += "\
        file://libselinux-drop-Wno-unused-but-set-variable.patch \
        file://libselinux-make-O_CLOEXEC-optional.patch \
        file://libselinux-make-SOCK_CLOEXEC-optional.patch \
        file://libselinux-define-FD_CLOEXEC-as-necessary.patch \
        file://0001-src-Makefile-fix-includedir-in-libselinux.pc.patch \
        "
