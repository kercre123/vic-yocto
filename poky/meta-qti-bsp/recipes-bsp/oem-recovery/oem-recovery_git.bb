inherit autotools-brokensep pkgconfig update-rc.d
PR = "r1"

DESCRIPTION = "OEM Recovery"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"
HOMEPAGE = "https://www.codeaurora.org/gitweb/quic/la?p=device/qcom/common.git"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://OTA/device/qcom/common/recovery/oem-recovery/"

S = "${WORKDIR}/OTA/device/qcom/common/recovery/oem-recovery/"

DEPENDS += "glib-2.0 virtual/kernel"

EXTRA_OECONF = "--with-glib --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
                --with-core-headers=${STAGING_INCDIR_NATIVE}"

PARALLEL_MAKE = ""
INITSCRIPT_NAME = "oem-recovery"
INITSCRIPT_PARAMS = "start 27 5 . stop 80 0 1 6 ."
