# Simple initramfs image artifact generation for tiny images.
DESCRIPTION = "Tiny image capable of booting a device. The kernel includes \
the Minimal RAM-based Initial Root Filesystem (initramfs), which finds the \
first 'init' program more efficiently."

LICENSE = "BSD-3-Clause-Clear"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-qti-bsp/files/common-licenses/\
${LICENSE};md5=48b43ba58d0f8e9ef3704313a46b7a43"

DEPENDS = "virtual/kernel"
PACKAGE_INSTALL += " ${@bb.utils.contains('DISTRO_FEATURES', 'vbleima', 'ima-cert-setup', '',d)}"
PACKAGE_INSTALL += " busybox"
PACKAGE_INSTALL += " mtd-utils-ubifs"
PACKAGE_INSTALL += " ${@bb.utils.contains_any('DISTRO_FEATURES', 'nad-avb nad-fde', 'cryptsetup', '',d)}"
PACKAGE_INSTALL += " ${@bb.utils.contains_any('DISTRO_FEATURES', 'nad-avb nad-fde', 'libdevmapper', '',d)}"

# Support reboot management
PACKAGE_INSTALL += " ${@bb.utils.contains('DISTRO_FEATURES', 'nad-prod', 'powerapp', '',d)}"
PACKAGE_INSTALL += " initramfs-init"

PACKAGE_EXCLUDE = " bash systemd-machine-units systemd busybox-udhcpc"
PR = "r0"
require include/machine-initramfs.inc
inherit qinitramfs

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

export IMAGE_BASENAME = "machine-initramfs"
IMAGE_LINGUAS = ""

# don't actually generate an image, just the artifacts needed for one
IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"

PACKAGE_ARCH = "${MACHINE_ARCH}"
