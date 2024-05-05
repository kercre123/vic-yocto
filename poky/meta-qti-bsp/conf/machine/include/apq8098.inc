BASEMACHINE ?= "apq8098"
require conf/machine/include/basemachine.inc

SOC_FAMILY ?= "msmcobalt"
include conf/machine/include/soc-family.inc

# Pull in security flags
require ${COREBASE}/meta-qti-bsp/conf/distro/include/security_flags.inc

require conf/machine/include/arm/arch-armv8.inc

PREFERRED_PROVIDER_virtual/kernel = "linux-msm"
PREFERRED_VERSION_linux-msm = "4.4"

MACHINE_DISPLAY_WIDTH_PIXELS ?= "480"
MACHINE_DISPLAY_HEIGHT_PIXELS ?= "640"

SERIAL_CONSOLE ?= "${@['', '115200 ttyMSM0'][d.getVar('VARIANT', True) == ('' or 'debug')]}"

DISTRO ?= "msm"

PREFERRED_VERSION_weston ?= "1.1.9%"

MACHINE_FEATURES += "kernel26 keyboard"

# drm driver
MACHINE_FEATURES += "drm"

PAGE_SIZE ?= '4096'

# Max supported inodes
EXTRA_IMAGECMD ?= "-N 2048"

# Sizes for EXT4 (in bytes)
SYSTEM_SIZE_EXT4 ?= "3000000000"
USERDATA_SIZE_EXT4 ?= "5000000000"
CACHE_SIZE_EXT4 ?= "65536000"
RECOVERYFS_SIZE_EXT4 ?= "136000000"

# Multilib configuration
# This sets any packages preprended with lib32- to be built with
# the armv7a tuning (32 bit) instead of 64 bit aarch.
require conf/multilib.conf
MULTILIBS = "multilib:lib32"
DEFAULTTUNE_virtclass-multilib-lib32 = "armv7a-neon"
TARGET_VENDOR_virtclass-multilib-lib32="-oe"
# Formats of root filesystem images.
IMAGE_FSTYPES += "ext4"
python __anonymous () {
    if (d.getVar('DEBUG', True) == '1'):
        d.setVar('DEBUG_BUILD', '1')
        d.setVar('INHIBIT_SYSROOT_STRIP', '1')
        d.appendVar('EXTRA_IMAGE_FEATURES', 'dbg-pkgs')

    if (d.getVar('NOPTMZ', True) == '1'):
        d.setVar('DEBUG_BUILD', '1')
        d.setVar('DEBUG_FLAGS', '-feliminate-unused-debug-types')
}
