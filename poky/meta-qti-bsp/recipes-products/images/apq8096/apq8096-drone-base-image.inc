#Include the drone settings

# Get base product settings
require apq8096-base-image.inc

IMAGE_INSTALL += "chrony"
IMAGE_INSTALL += "chronyc"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "av-frameworks"
IMAGE_INSTALL += "openssh"
IMAGE_INSTALL += "attr"
IMAGE_INSTALL += "lib32-libbt-vendor"
IMAGE_INSTALL += "lib32-fluoride"
IMAGE_INSTALL += "lib32-bt-property"
IMAGE_INSTALL += "lib32-bt-app"
IMAGE_INSTALL += "lib32-ftm"
IMAGE_INSTALL += "openvpn"
IMAGE_INSTALL += "system-core-post-boot"
IMAGE_INSTALL += "perl"
IMAGE_INSTALL += "opkg"
IMAGE_INSTALL += "libarchive"
IMAGE_INSTALL += "libpam"
IMAGE_INSTALL += "bzip2"
