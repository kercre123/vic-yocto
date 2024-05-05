inherit autotools-brokensep linux-kernel-base pkgconfig

DESCRIPTION = "CNSS"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r2"

DEPENDS = "libcutils libnl liblog"

FILESPATH =+ "${WORKSPACE}/hardware/qcom/:"

SRC_URI = "file://wlan/cld80211-lib/"

S = "${WORKDIR}/wlan/cld80211-lib"

CFLAGS += "-I ${STAGING_INCDIR}/libnl3"
CFLAGS += "-I ${WORKSPACE}/system/core/include/"
