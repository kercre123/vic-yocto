inherit autotools

DESCRIPTION = "Tinyalsa Library"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PR = "r1"

SRCREV = "1369a0ff9979cfbdaa0ca88e4696265655cd198b"
SRC_URI = "git://git.codelinaro.org/clo/le/platform/external/tinyalsa.git;protocol=https;branch=caf_migration/github/master \
           file://Makefile.am \
           file://configure.ac \
           file://tinyalsa.pc.in \
           file://0001-tinyalsa-Added-avail_min-member.patch \
           file://0001-Add-PCM_FORMAT_INVALID-constant.patch \
           file://0001-pcm-add-support-to-set-silence_size.patch \
           file://0001-tinyalsa-Enable-compilation-with-latest-TinyALSA.patch \
           file://0001-tinyalsa-add-mixer_read-api-to-read-event-informatio.patch"

S = "${WORKDIR}"

EXTRA_OEMAKE = "DEFAULT_INCLUDES=-I${WORKDIR}/git/include/"

DEPENDS = "libcutils"
