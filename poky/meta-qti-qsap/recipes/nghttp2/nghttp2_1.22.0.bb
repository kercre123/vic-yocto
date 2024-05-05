DESCRIPTION = "HTTP Version 2 client library in C"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=764abdf30b2eadd37ce47dcbce0ea1ec \
                    file://doc/_exts/sphinxcontrib/LICENSE.rubydomain;md5=667c3e266c41ac5129a4478ad682b1c3"

SRC_URI[md5sum] = "99d7d2c8073be8dd8801e453a4307037"
SRC_URI[sha256sum] = "f14af22f14107901ea6077413f1a387948bf11cdaa4613ba361a0e3e8cacbbe7"

SRC_URI = "https://github.com/nghttp2/nghttp2/releases/download/v${PV}/nghttp2-${PV}.tar.xz"

inherit autotools pkgconfig cmake

BBCLASSEXTEND = "native nativesdk"
