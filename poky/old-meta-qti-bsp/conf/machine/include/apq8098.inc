BASEMACHINE ?= "apq8098"
require conf/machine/include/basemachine.inc

SOC_FAMILY ?= "msmcobalt"
include conf/machine/include/soc-family.inc

# Uncomment the following line to enable the hard floating point abi. Note that
# this breaks some binary libraries and 3D (neither of which ship with
# meta-yocto). For maximum compatibility, leave this disabled.
#DEFAULTTUNE ?= "cortexa8hf-neon"

require conf/machine/include/arm/arch-armv8.inc

BBMASK = ".*/*/recipes-kernel/linux-quic"
BBMASK .= "|.*/*/recipes-security/libselinux/"

MACHINE_DISPLAY_WIDTH_PIXELS ?= "480"
MACHINE_DISPLAY_HEIGHT_PIXELS ?= "640"

SERIAL_CONSOLE = "115200 ttyMSM0"

DISTRO ?= "msm"

MACHINE_FEATURES += "kernel26 keyboard"

PAGE_SIZE ?= '4096'

# Max supported inodes
EXTRA_IMAGECMD ?= "-N 2048"

# Sizes for EXT4 (in bytes)
SYSTEM_SIZE_EXT4 ?= "3000000000"
USERDATA_SIZE_EXT4 ?= "5000000000"
CACHE_SIZE_EXT4 ?= "65536000"

# Multilib configuration
# This sets any packages preprended with lib32- to be built with
# the armv7a tuning (32 bit) instead of 64 bit aarch.
require conf/multilib.conf
MULTILIBS = "multilib:lib32"
DEFAULTTUNE_virtclass-multilib-lib32 = "armv7a"

# Formats of root filesystem images.
IMAGE_FSTYPES += "ext4"
