inherit autotools-brokensep module qperf
DESCRIPTION = "Code Aurora TCP Splice"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=801f80980d171dd6425610833a22dbe6"

PR = "r0"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://tcp-splice/"

S = "${WORKDIR}/tcp-splice/"

do_install() {
    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/net
    install -m 0644 ${S}tcp_splice.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/net
}
