inherit autotools-brokensep pkgconfig

DESCRIPTION = "GPS Loc Stub"
PR = "r1"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://hardware/qcom/gps/utils/platform_lib_abstractions/loc_stub/"
S = "${WORKDIR}/hardware/qcom/gps/utils/platform_lib_abstractions/loc_stub"
DEPENDS = "glib-2.0 liblog"
EXTRA_OECONF = "--with-glib"
