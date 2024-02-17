DESCRIPTION = "Small initramfs image based on poky/meta/core-recipes/core-image-minimal-initramfs - directly boots into dm-verity rootfs on emmc."

PACKAGE_INSTALL = "busybox udev base-passwd ${ROOTFS_BOOTSTRAP_INSTALL} lvm2"

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

export IMAGE_BASENAME = "core-image-anki-initramfs"
IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
inherit core-image qperf

IMAGE_ROOTFS_SIZE = "8192"

BAD_RECOMMENDATIONS += "busybox-syslog"
