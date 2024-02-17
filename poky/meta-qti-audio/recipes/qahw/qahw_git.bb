inherit autotools pkgconfig

DESCRIPTION = "qahw"
SECTION = "multimedia"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILESPATH =+ "${WORKSPACE}/:"
SRC_URI  = "file://hardware/qcom/audio/qahw/"

S = "${WORKDIR}/hardware/qcom/audio/qahw/"
PR = "r0"

DEPENDS += "libhardware liblog libcutils media-headers audio-route audio-utils glib-2.0"

EXTRA_OECONF = "--with-glib"
EXTRA_OECONF_append_apq8009 = " BOARD_SUPPORTS_SVA_AUDIO_CONCURRENCY=true"
EXTRA_OECONF_append_apq8017 = " BOARD_SUPPORTS_SVA_AUDIO_CONCURRENCY=true"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""
