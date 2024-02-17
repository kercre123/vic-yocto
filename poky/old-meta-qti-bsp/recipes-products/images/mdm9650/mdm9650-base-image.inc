# Rootfs creation.

DEPENDS += "lk"

# Formats of root filesystem images.
IMAGE_FSTYPES += "ext4 ubi"

IMAGE_INSTALL += "packagegroup-core-boot"

IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# The MSM Linux minimal boot image files.

IMAGE_INSTALL += "alx"
IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "ddclient"
IMAGE_INSTALL += "gdbserver"
IMAGE_INSTALL += "gdb"
IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "i2c-tools"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "jquery"
IMAGE_INSTALL += "jquery-ui"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "libnl"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "libiio"
IMAGE_INSTALL += "libiio-iiod"
IMAGE_INSTALL += "libiio-tests"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libstdc++"
IMAGE_INSTALL += "libxml2"
IMAGE_INSTALL += "minidlna"
IMAGE_INSTALL += "openssl"
IMAGE_INSTALL += "pimd"
IMAGE_INSTALL += "powerapp"
IMAGE_INSTALL += "powerapp-powerconfig"
IMAGE_INSTALL += "powerapp-reboot"
IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "perl"
IMAGE_INSTALL += "sigma-dut"
IMAGE_INSTALL += "spitools"
IMAGE_INSTALL += "start-scripts-find-partitions"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "system-conf"
IMAGE_INSTALL += "tcpdump"
IMAGE_INSTALL += "wireless-tools"

IMAGE_PREPROCESS_COMMAND += "echo ${MACHINE} >> ${IMAGE_ROOTFS}/target;"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)

do_makesystem() {
    make_ext4fs -s -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${MACHINE}-${PRODUCT}-sysfs.ext4 ${IMAGE_ROOTFS}
    make_ext4fs -s -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${MACHINE}-${PRODUCT}-systemrw.ext4
}
