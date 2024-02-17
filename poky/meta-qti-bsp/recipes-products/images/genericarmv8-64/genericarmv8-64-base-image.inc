# Rootfs creation.
IMAGE_INSTALL += "packagegroup-core-boot"
#IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# The MSM Linux minimal boot image files.
IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "glib-2.0"
#IMAGE_INSTALL += "gdbserver"
IMAGE_INSTALL += "gdb"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "libnl"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libstdc++"
IMAGE_INSTALL += "libxml2"
IMAGE_INSTALL += "openssl"
IMAGE_INSTALL += "perl"
IMAGE_INSTALL += "powerapp"
IMAGE_INSTALL += "powerapp-powerconfig"
IMAGE_INSTALL += "powerapp-reboot"
IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "start-scripts-find-partitions"
#IMAGE_INSTALL += "start-scripts-firmware-links"
#IMAGE_INSTALL += "system-core-adbd"
#IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "volatile-binds"
#IMAGE_INSTALL += "edk2"
IMAGE_INSTALL += "wireless-tools"
IMAGE_INSTALL += "coreutils"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)
do_makesystem() {
    make_ext4fs -s ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
}
