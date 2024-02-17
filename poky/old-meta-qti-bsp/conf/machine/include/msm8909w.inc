BASEMACHINE ?= "apq8009"
require conf/machine/include/basemachine.inc

SOC_FAMILY ?= "msm8909"
include conf/machine/include/soc-family.inc

# Uncomment the following line to enable the hard floating point abi. Note that
# this breaks some binary libraries and 3D (neither of which ship with
# meta-yocto). For maximum compatibility, leave this disabled.
#DEFAULTTUNE ?= "cortexa8hf-neon"

require conf/machine/include/apq8009.inc

MACHINE_DISPLAY_WIDTH_PIXELS ?= "320"
MACHINE_DISPLAY_HEIGHT_PIXELS ?= "320"

EXTRA_IMAGECMD ?= "-N 2048"

# Sizes for EXT4 (in bytes)
SYSTEM_SIZE_EXT4 ?= "536870912"
USERDATA_SIZE_EXT4 ?= "200000000"
CACHE_SIZE_EXT4 ?= "65536000"

# Formats of root filesystem images.
IMAGE_FSTYPES = "ext4"
