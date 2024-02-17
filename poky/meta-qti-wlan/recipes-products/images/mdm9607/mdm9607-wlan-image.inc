# WLAN open source Packages
#IMAGE_INSTALL += "qcacld-hl"
IMAGE_INSTALL += "hostap-daemon-qcacld"
#IMAGE_INSTALL += "${@base_conditional('QMI_DIR_PRESENT', 'True', 'wpa-supplicant-qcacld', '', d)}"
