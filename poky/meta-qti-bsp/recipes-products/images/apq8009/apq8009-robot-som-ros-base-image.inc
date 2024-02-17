#Include the robot settings

# Get base product settings
require apq8009-base-image.inc
# These two lines below is a workaround to remove init-mss,
# since packagegroup-ss-mgr just include reboot-daemon and init-mss
IMAGE_INSTALL_remove = "packagegroup-ss-mgr"
IMAGE_INSTALL += "reboot-daemon"
IMAGE_INSTALL += "e2fsprogs"
IMAGE_INSTALL += "start-scripts-userfs-reset"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util', '', d)}"
IMAGE_INSTALL += "chrony"
IMAGE_INSTALL += "chronyc"

IMAGE_INSTALL += "openssh"
IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','robot-rome', 'iw', '', d)}"
IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','robot-rome', 'libcap', '', d)}"
IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','robot-rome', 'attr', '', d)}"
IMAGE_INSTALL += "mtd-utils-ubifs"
IMAGE_INSTALL += "kernel-modules"
# Following packages support audio-dlkm enabled

AUDIOMODULE_FLAG = "${@bb.utils.contains('DISTRO_FEATURES', 'audio-dlkm', 'True', 'False', d)}"
IMAGE_INSTALL += "${@oe.utils.conditional('AUDIOMODULE_FLAG', 'True', bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-audiodlkm', 'audiodlkm', d), '', d)}"
