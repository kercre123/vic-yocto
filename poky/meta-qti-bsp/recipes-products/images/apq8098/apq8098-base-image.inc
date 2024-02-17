# Rootfs creation.

DEPENDS += "edk2"


IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# The MSM Linux minimal boot image files.

IMAGE_INSTALL += "alx"

IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"

DEPENDS += "gdb-cross-aarch64"

IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "start-scripts-find-partitions"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-leprop"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "trace-cmd"
IMAGE_INSTALL += "wireless-tools"
IMAGE_INSTALL += "pixman"
IMAGE_INSTALL += "libinput"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)
do_makesystem() {
    make_ext4fs -s ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
}
