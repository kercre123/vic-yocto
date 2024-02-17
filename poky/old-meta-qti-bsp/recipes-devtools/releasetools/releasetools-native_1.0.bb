inherit native

DESCRIPTION = "releasetools used for OTA"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

PR = "r0"

FILESPATH =+ "${WORKSPACE}/android_compat/build/tools/:"

SRC_URI   = "file://releasetools/"
SRC_URI  += "file://full_ota.sh"
SRC_URI  += "file://incremental_ota.sh"

S = "${WORKDIR}/releasetools"


do_configure_append() {
    mv ${WORKDIR}/full_ota.sh ${S}
    chmod 755 ${S}/full_ota.sh
    mv ${WORKDIR}/incremental_ota.sh ${S}
    chmod 755 ${S}/incremental_ota.sh
}

do_compile[noexec] = "1"
do_install[noexec] = "1"

