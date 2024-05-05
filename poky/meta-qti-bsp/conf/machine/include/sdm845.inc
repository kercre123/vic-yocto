BASEMACHINE ?= "sdm845"
require conf/machine/include/basemachine.inc

SOC_FAMILY ?= "sdm845"
include conf/machine/include/soc-family.inc

# Pull in security flags
require ${COREBASE}/meta-qti-bsp/conf/distro/include/security_flags.inc

require conf/machine/include/arm/arch-armv8.inc

PREFERRED_PROVIDER_virtual/kernel = "linux-msm"
PREFERRED_VERSION_linux-msm = "4.9"

PREFERRED_PROVIDER_virtual/egl ?= "adreno"
PREFERRED_PROVIDER_virtual/libgles1 ?= "adreno"
PREFERRED_PROVIDER_virtual/libgles2 ?= "adreno"

SERIAL_CONSOLE ?= "${@['', '115200 ttyMSM0'][d.getVar('VARIANT', True) == ('' or 'debug')]}"

DISTRO ?= "msm"

DISTRO_FEATURES += "persist-image"
DISTRO_FEATURES += "ab-boot-support"
DISTRO_FEATURES += "gbm"

# drm driver
MACHINE_FEATURES += "drm"

PAGE_SIZE ?= '4096'

# Max supported inodes
EXTRA_IMAGECMD ?= "-N 2048"

MACHINE_MNT_POINTS = "/firmware /persist /cache /dsp /bt_firmware"

# Sizes for EXT4 (in bytes)
SYSTEM_SIZE_EXT4 ?= "3221225472"
SYSTEMRW_SIZE_EXT4 ?= "8388608"
USERDATA_SIZE_EXT4 ?= "5000000000"
CACHE_SIZE_EXT4 ?= "65536000"
RECOVERYFS_SIZE_EXT4 ?= "136000000"

# Enable LE Verified Boot
DISTRO_FEATURES_append = " vble"

#Enable SDK feature
# SDK_IMAGE ?= "1"

#Enable Recovery image generation
RECOVERY_IMAGE ?= "0"

# Formats of root filesystem images.
IMAGE_FSTYPES += "ext4"
