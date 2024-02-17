# WLAN open source Packages
IMAGE_INSTALL += "wcnss"
IMAGE_INSTALL += "wlan-opensource"

# if 'wifi' is set as a feature, we get Poky's own wpa_supplicant
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','wifi', '', 'wpa-supplicant-qcacld', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','wifi', '', 'hostap-daemon-qcacld', d)}"
#if ${@bb.utils.contains('DISTRO_FEATURES','wifi','false','true',d)}; then
#    IMAGE_INSTALL += "wpa-supplicant-qcacld"
#    IMAGE_INSTALL += "hostap-daemon-qcacld"
#fi

IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','drone', 'qcacld-hl', '', d)}"
IMAGE_INSTALL_remove = "${@oe.utils.conditional('PRODUCT','drone', 'wlan-opensource', '', d)}"
IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','drone', 'wlan-services', '', d)}"
