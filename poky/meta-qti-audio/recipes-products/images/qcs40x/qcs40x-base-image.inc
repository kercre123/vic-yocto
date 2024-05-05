# Rootfs creation.

IMAGE_INSTALL += "init-audio"

# Following packages support only 32-bit compilation.
# When 32-bit multilib is enabled, compile them with lib32- prefix.

IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-audiohal', 'audiohal', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-tinyalsa', 'tinyalsa', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-tinycompress', 'tinycompress', d)}"
#IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-encoders', 'encoders', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-qahw', 'qahw', d)}"
AUDIOMODULE_FLAG = "${@bb.utils.contains('DISTRO_FEATURES', 'audio-dlkm', 'True', 'False', d)}"
IMAGE_INSTALL += "${@oe.utils.conditional('AUDIOMODULE_FLAG', 'True', bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-audiodlkm', 'audiodlkm', d), '', d)}"
