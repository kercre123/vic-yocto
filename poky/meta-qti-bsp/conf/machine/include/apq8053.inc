BASEMACHINE ?= "apq8053"
require conf/machine/include/basemachine.inc

SOC_FAMILY ?= "msm8953"
include conf/machine/include/soc-family.inc

# Pull in security flags
require ${COREBASE}/meta-qti-bsp/conf/distro/include/security_flags.inc

require conf/machine/include/arm/arch-armv8.inc

PREFERRED_PROVIDER_virtual/kernel = "linux-msm"
PREFERRED_VERSION_linux-msm = "4.9"

PREFERRED_PROVIDER_virtual/egl ?= "adreno"
PREFERRED_PROVIDER_virtual/libgles1 ?= "adreno"
PREFERRED_PROVIDER_virtual/libgles2 ?= "adreno"

MACHINE_DISPLAY_WIDTH_PIXELS ?= "480"
MACHINE_DISPLAY_HEIGHT_PIXELS ?= "640"

SERIAL_CONSOLE ?= "${@['', '115200 ttyMSM0'][d.getVar('VARIANT', True) == ('' or 'debug')]}"

DISTRO ?= "msm"

# Change default jpeg provider to turbo jpeg
PREFERRED_PROVIDER_jpeg = "libjpeg-turbo"

# iproute2 bump version to 4.1.1
PREFERRED_VERSION_iproute2 = "4.1.1"

DISTRO_FEATURES += "persist-image"

MACHINE_FEATURES += "kernel26 keyboard"

PAGE_SIZE ?= '4096'

# Max supported inodes
EXTRA_IMAGECMD ?= "-N 2048"

MACHINE_MNT_POINTS = "/firmware /persist /cache /dsp"

# Sizes for EXT4 (in bytes)
SYSTEM_SIZE_EXT4 ?= "536870912"
SYSTEMRW_SIZE_EXT4 ?= "8388608"
USERDATA_SIZE_EXT4 ?= "1048576000"
CACHE_SIZE_EXT4 ?= "262144000"
RECOVERYFS_SIZE_EXT4 ?= "136000000"

#Enable SDK feature
SDK_IMAGE ?= "1"

#Enable Recovery image generation
RECOVERY_IMAGE ?= "0"

# Formats of root filesystem images.
IMAGE_FSTYPES += "ext4"

#Extra fsconfig related options for ota package generator scripts.
OTA_EXTRA_OPTS ?= "1"

# Support A/B schema for eMMC partitions
DISTRO_FEATURES_append = " ab-boot-support"

# Enable face recognition support
DISTRO_FEATURES_append = " face-recognition"

# Enable LE Verified Boot
DISTRO_FEATURES_append = " vble"
