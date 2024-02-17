require ${COREBASE}/meta-qti-bsp/recipes-products/images/common/common-base-image.inc

#compile abl
RDEPENDS  += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib64','lib64-edk2','edk2',d)}"

#install userspace packages
IMAGE_INSTALL += "av-frameworks"
IMAGE_INSTALL += "binder"
IMAGE_INSTALL += "libion"
IMAGE_INSTALL += "libhardware"
IMAGE_INSTALL += "media-headers"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util', '', d)}"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "iw"

# Install recovery and its libs in system image if A/B boot is supported
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'recovery-ab-bin', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'recovery-ab', '', d)}"
