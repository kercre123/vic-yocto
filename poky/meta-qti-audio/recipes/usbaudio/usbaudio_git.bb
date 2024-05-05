inherit autotools pkgconfig

DESCRIPTION = "usbaudio"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI   = "file://hardware/libhardware/modules/usbaudio/"
S = "${WORKDIR}/hardware/libhardware/modules/usbaudio/"

PR = "r0"

DEPENDS = "tinyalsa system-media libhardware"

FILES_${PN} += "${libdir}/*.so"
INSANE_SKIP_${PN} = "dev-deps"
