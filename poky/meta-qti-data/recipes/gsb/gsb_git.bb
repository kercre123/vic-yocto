inherit autotools-brokensep module qperf
DESCRIPTION = "Generic Software Bridge Driver"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=801f80980d171dd6425610833a22dbe6"

PR = "r0"

FILESPATH =+ "${WORKSPACE}/data-kernel/drivers:"
SRC_URI = "file://generic-sw-bridge"
S = "${WORKDIR}/generic-sw-bridge/"

do_install() {
    module_do_install
}
