KBRANCH ?= "anki/msm/linux-3.18"

require recipes-kernel/linux/linux-yocto.inc

inherit externalsrc qperf

PN = "linux-quic"

SRCREV ?= "${AUTOREV}"
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
EXTERNALSRC = "${WORKSPACE}/kernel/msm-3.18"

LINUX_VERSION_EXTENSION_msm-perf = "-perf"
LINUX_VERSION_EXTENSION_msm-user = "-perf"
LINUX_VERSION_EXTENSION_msm= ""

FILESEXTRAPATHS_prepend := "${THISDIR}/meta/cfg/${LINUX_KERNEL_TYPE}/${KMACHINE}:"
SRC_URI += "file://defconfig"

LINUX_VERSION ?= "3.18.66"

DEPENDS += "openssl-native util-linux-native"
DEPENDS += "dtbtool-native mkbootimg-native"
DEPENDS_apq8096 += "mkbootimg-native dtc-native"
PACKAGES = "kernel kernel-base kernel-vmlinux kernel-dev kernel-modules"
RDEPENDS_kernel-base = ""

PV = "${LINUX_VERSION}+git${SRCPV}"

KMETA = "kernel-meta"
KCONF_BSP_AUDIT_LEVEL = "2"

KERNEL_DEVICETREE = "qcom/msm8909-anki.dtb"
COMPATIBLE_MACHINE = "apq8009-robot"

do_install_prepend() {
    oe_runmake headers_install INSTALL_HDR_PATH=${STAGING_KERNEL_BUILDDIR}/usr
}
