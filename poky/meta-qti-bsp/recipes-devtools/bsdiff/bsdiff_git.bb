inherit autotools pkgconfig native

PR = "r4"

DESCRIPTION = "bsdiff tool from Android"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
BSD;md5=3775480a712fc46a69647678acb234cb"

DEPENDS += "bzip2-replacement-native libdivsufsort-native"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://OTA/external/bsdiff/"

S = "${WORKDIR}/OTA/external/bsdiff/"

EXTRA_OECONF = "--with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
                --with-core-headers=${STAGING_INCDIR_NATIVE}"

BBCLASSEXTEND = "native"
