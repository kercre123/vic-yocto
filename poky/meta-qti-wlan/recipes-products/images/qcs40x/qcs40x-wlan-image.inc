# WLAN open source Packages
IMAGE_INSTALL += "${@bb.utils.contains('MACHINE_FEATURES', 'wlan', 'packagegroup-wlan', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MACHINE_FEATURES', 'wlan-sdio', 'packagegroup-wlan-sdio', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'wlan-perf', 'packagegroup-wlan-debug', '', d)}"
