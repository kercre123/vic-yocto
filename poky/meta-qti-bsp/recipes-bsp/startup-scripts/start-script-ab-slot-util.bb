DESCRIPTION = "Installs script for AB slot support"
HOMEPAGE = "http://codeaurora.org"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=550794465ba0ec5312d6919e203a55f9"

SRC_URI +="file://getslotsuffix.sh"

S = "${WORKDIR}/"

PR = "r0"

inherit useradd

do_install() {
    install -m 0754 ${WORKDIR}/getslotsuffix.sh -D ${D}${bindir}/getslotsuffix
}
