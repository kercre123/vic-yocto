# Rootfs creation.

IMAGE_INSTALL += "init-audio"

# Following packages support only 32-bit compilation.
# When 32-bit multilib is enabled, compile them with lib32- prefix.

IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-audiohal', 'audiohal', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-tinyalsa', 'tinyalsa', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-encoders', 'encoders', d)}"