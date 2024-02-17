inherit autotools

DESCRIPTION = "MSM7K"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PR = "r2"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://hardware/msm7k"
S = "${WORKDIR}/hardware/msm7k"

DEPENDS = "glib-2.0"

EXTRA_OECONF = "--with-glib"

