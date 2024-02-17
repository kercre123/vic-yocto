# wlan open source Packages
include ${BASEMACHINE}/${BASEMACHINE}-wlan-image.inc
PACKAGE_EXCLUDE_append_mdm9607 = " qcacld-hl wpa-supplicant-qcacld hostap-daemon-qcacld"
