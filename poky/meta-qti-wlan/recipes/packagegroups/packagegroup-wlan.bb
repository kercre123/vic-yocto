SUMMARY = "WLAN opensource package groups"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

PROVIDES = "${PACKAGES}"
PACKAGES = " \
    packagegroup-wlan \
    ${@bb.utils.contains('MACHINE_FEATURES', 'wlan-sdio', 'packagegroup-wlan-sdio', '', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wlan-perf', 'packagegroup-wlan-debug', '', d)} \
    "

RDEPENDS_packagegroup-wlan = " \
    qcacld32-ll \
    hostap-daemon-qcacld \
    wpa-supplicant-qcacld \
    wpa-supplicant-8-lib \
    cld80211-lib \
    wlan-conf \
    wireless-tools \
    "
RDEPENDS_packagegroup-wlan-sdio = " \
    qcacld-hl \
    "
RDEPENDS_packagegroup-wlan-debug = " \
    qcacld32-ll-nf-debug \
    "
