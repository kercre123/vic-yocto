inherit native

DESCRIPTION = "releasetools used for OTA"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

PR = "r0"

DEPENDS += "libselinux libpcre2 liblog fsconfig-native applypatch-native libdivsufsort-native bsdiff-native"

FILESPATH =+ "${WORKSPACE}/OTA/build/tools/:"

SRC_URI   = "file://releasetools/"
SRC_URI  += "file://full_ota.sh"
SRC_URI  += "file://incremental_ota.sh"

S = "${WORKDIR}/releasetools"


do_configure_append() {
    cp ${WORKDIR}/full_ota.sh ${S}
    chmod 755 ${S}/full_ota.sh
    cp ${WORKDIR}/incremental_ota.sh ${S}
    chmod 755 ${S}/incremental_ota.sh
}

do_compile[noexec] = "1"
do_install[noexec] = "1"
