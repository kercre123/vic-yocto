inherit autotools-brokensep pkgconfig

DESCRIPTION = "encoders"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r0"

FILESPATH   =+ "${WORKSPACE}/:"
SRC_URI     =  "file://hardware/qcom/audio/mm-audio/"

S = "${WORKDIR}/hardware/qcom/audio/mm-audio/"
AUDIO_KERNEL_HEADERS="${STAGING_KERNEL_BUILDDIR}/audio-kernel"
CFLAGS += "-I${AUDIO_KERNEL_HEADERS}"

EXTRA_OECONF_append += "--with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"
EXTRA_OECONF_append += "--with-glib"
EXTRA_OECONF_append = " --with-audio-kernel-headers=${AUDIO_KERNEL_HEADERS}"

DEPENDS += "glib-2.0 media"
DEPENDS_append = "${@bb.utils.contains('DISTRO_FEATURES', 'audio-dlkm', ' audiodlkm', '', d)}"
DEPENDS_append = "${@bb.utils.contains('COMBINED_FEATURES', 'qti-audio', ' liblog libcutils', ' system-core', d)}"

do_configure[depends] += "audiodlkm:do_install"

RDEPENDS_${PN} = "media"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""
