inherit autotools-brokensep pkgconfig

DESCRIPTION = "Data Services Open Source"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r4"

DEPENDS += "virtual/kernel glib-2.0"

EXTRA_OECONF = "--with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include --with-glib"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://data-oss"
S = "${WORKDIR}/data-oss"
