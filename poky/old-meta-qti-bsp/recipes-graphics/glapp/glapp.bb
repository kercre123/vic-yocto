DESCRIPTION = "glapp"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
PR = "r0"

DEPENDS = "adreno"

FILESPATH =+ "${WORKSPACE}"
SRC_DIR = "${WORKSPACE}/frameworks/native/opengl/tests/glapp/"
S = "${WORKDIR}/frameworks/native/opengl/tests/glapp/"

inherit autotools qcommon
