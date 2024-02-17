inherit pkgconfig native autotools

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
HOMEPAGE = "https://www.codeaurora.org/gitweb/quic/la?p=platform/bootable/recovery.git"

PR = "r1"

DEPENDS += "bzip2-replacement-native"
RDEPENDS_${PN} = "zlib"

FILESPATH =+ "${WORKSPACE}/OTA/recovery/:"
SRC_URI   = "file://applypatch"

S = "${WORKDIR}/applypatch"
