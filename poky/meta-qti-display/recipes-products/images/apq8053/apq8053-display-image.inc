# DISPLAY Open source Packages

# Display HAL can compile for both 32bit and 64bit.
# When 32-bit multilib is enabled, compile both variants.

IMAGE_INSTALL += "display-hal"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-display-hal', '', d)}"
