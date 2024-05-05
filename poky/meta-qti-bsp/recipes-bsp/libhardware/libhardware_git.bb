inherit autotools pkgconfig

DESCRIPTION = "hardware libhardware headers"
HOMEPAGE = "http://codeaurora.org/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI   = "file://hardware/libhardware/"
S = "${WORKDIR}/hardware/libhardware"

PR = "r6"

DEPENDS += "libcutils liblog system-core"

EXTRA_OECONF_append_apq8053 = " --enable-sensors"
EXTRA_OECONF_append_concam = " --enable-camera"
EXTRA_OECONF_append_sdm845 = " --enable-sensors"
EXTRA_OECONF_append_sdm845 = " --enable-camera"
EXTRA_OECONF_append_robot-som = " --enable-camera"
