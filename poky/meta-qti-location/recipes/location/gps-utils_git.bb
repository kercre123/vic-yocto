inherit autotools-brokensep qcommon pkgconfig

DESCRIPTION = "GPS Utils"
PR = "r1"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

SRC_DIR = "${WORKSPACE}/hardware/qcom/gps/utils/"
S = "${WORKDIR}/hardware/qcom/gps/utils"

DEPENDS = "glib-2.0 loc-pla libcutils"
EXTRA_OECONF = "--with-glib"

