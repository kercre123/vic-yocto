
inherit native autotools-brokensep pkgconfig

PR = "r1"

DESCRIPTION = "libdivsufsort from Android"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://OTA/external/libdivsufsort/"

S = "${WORKDIR}/OTA/external/libdivsufsort/"

EXTRA_OECONF = "--with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
                --with-core-headers=${STAGING_INCDIR_NATIVE}"

BBCLASSEXTEND = "native"
