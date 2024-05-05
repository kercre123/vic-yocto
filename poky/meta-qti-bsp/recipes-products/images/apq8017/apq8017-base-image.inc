# Rootfs creation.

DEPENDS += "lib32-lk"


IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# The MSM Linux minimal boot image files.

IMAGE_INSTALL += "rmtstorage"

IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
DEPENDS += "gdb-cross-aarch64"
IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "lib32-libnl"
IMAGE_INSTALL += "libxml2"

IMAGE_INSTALL += "start-scripts-find-partitions"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-leprop"
IMAGE_INSTALL += "system-core-usb"

IMAGE_INSTALL += "system-core-post-boot"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "systemd-machine-units"
IMAGE_INSTALL += "wireless-tools"
IMAGE_INSTALL += "volatile-binds"

IMAGE_INSTALL += "ca-certificates"

IMAGE_INSTALL += "e2fsprogs"
IMAGE_INSTALL += "e2fsprogs-e2fsck"
IMAGE_INSTALL += "e2fsprogs-resize2fs"

IMAGE_INSTALL += "start-scripts-userfs-reset"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)
do_makesystem() {
	make_ext4fs -s ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
        make_ext4fs -a /systemrw ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMRWIMAGE_TARGET}
}
