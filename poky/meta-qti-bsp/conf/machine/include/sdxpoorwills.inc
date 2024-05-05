BASEMACHINE ?= "sdxpoorwills"
require conf/machine/include/basemachine.inc

SOC_FAMILY ?= "mdm9x50"
include conf/machine/include/soc-family.inc

# Pull in security flags
require ${COREBASE}/meta-qti-bsp/conf/distro/include/security_flags.inc

include conf/machine/include/tune-cortexa8.inc

PREFERRED_PROVIDER_virtual/kernel = "linux-msm"
PREFERRED_VERSION_linux-msm = "4.9"

MACHINE_DISPLAY_WIDTH_PIXELS ?= "480"
MACHINE_DISPLAY_HEIGHT_PIXELS ?= "640"

SERIAL_CONSOLES ?= "115200;ttyMSM0"

DISTRO ?= "mdm"

DISTRO_FEATURES_append = " persist-volume"

#Disable ssd listener
DISTRO_FEATURES_append = " no-ssd-listener-support"

#Disable rpmb listner
DISTRO_FEATURES_append = " no-rpmb-listener-support"

MACHINE_FEATURES += "kernel26 keyboard ipa-offload"

PAGE_SIZE ?= '4096'

# Max supported inodes
EXTRA_IMAGECMD ?= "-N 2048"

#UBINIZE Args
UBINIZE_ARGS ?= "-m 4096 -p 256KiB -s 4096"
#MKUBIFS Args
MKUBIFS_ARGS ?= "-m 4096 -e 253952 -c 2146 -F"
#Volume sizes
SYSTEM_VOLUME_SIZE ?= "90MiB"
CACHE_VOLUME_SIZE ?= "70MiB"
SYSTEMRW_VOLUME_SIZE ?= "6MiB"
PERSIST_VOLUME_SIZE ?= "6MiB"

# Sizes for EXT4 (in bytes)
SYSTEM_SIZE_EXT4 ?= "100000000"
SYSTEMRW_SIZE_EXT4 ?= "8388608"
USERDATA_SIZE_EXT4 ?= "2000000000"
CACHE_SIZE_EXT4 ?= "80000000"

# Enable LE Verified Boot
DISTRO_FEATURES_append = " vble"

# Formats of root filesystem images.
IMAGE_FSTYPES += "ubi"
