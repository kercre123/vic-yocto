inherit autotools pkgconfig

DESCRIPTION = "Build LE libbase"
HOMEPAGE = "http://developer.android.com/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

PR = "r1"

DEPENDS += "libcutils libselinux"

BBCLASSEXTEND = "native"

FILESPATH =+ "${WORKSPACE}/system/core/:"
SRC_URI   = "file://base"

S = "${WORKDIR}/base"

EXTRA_OECONF += "--with-core-sourcedir=${WORKSPACE}/system/core"

FILES_${PN}-dbg    = "${libdir}/.debug/libbase.*"
FILES_${PN}        = "${libdir}/libbase.so.* ${libdir}/pkgconfig/*"
FILES_${PN}-dev    = "${libdir}/libbase.so ${libdir}/libbase.la ${includedir}"
