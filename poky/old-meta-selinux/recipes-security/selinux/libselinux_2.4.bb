include selinux_20150202.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://LICENSE;md5=84b4d2c6ef954a2d4081e775a270d0d0"

SRC_URI[md5sum] = "d19af2a367a81fb00bedc1b381694995"
SRC_URI[sha256sum] = "46043091f4c5ba4f43e8d3715f30d665a2d571c9126c1f03945c9ea4ed380f7b"

SRC_URI += "\
        file://libselinux-drop-Wno-unused-but-set-variable.patch \
        file://libselinux-make-O_CLOEXEC-optional.patch \
        file://libselinux-make-SOCK_CLOEXEC-optional.patch \
        file://libselinux-define-FD_CLOEXEC-as-necessary.patch \
        file://libselinux-get-pywrap-depends-on-selinux.py.patch \
        "
