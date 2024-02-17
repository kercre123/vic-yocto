require apq8009-base-image.inc

IMAGE_INSTALL_remove = "packagegroup-ss-mgr"
IMAGE_INSTALL += "reboot-daemon"
IMAGE_INSTALL += "e2fsprogs"
IMAGE_INSTALL += "start-scripts-userfs-reset"
IMAGE_INSTALL += "chrony"
IMAGE_INSTALL += "chronyc"

IMAGE_INSTALL += "openssh"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "libcap"
IMAGE_INSTALL += "attr"
IMAGE_INSTALL += "mtd-utils-ubifs"
