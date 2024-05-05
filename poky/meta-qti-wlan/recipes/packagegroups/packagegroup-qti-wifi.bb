SUMMARY = "QTI WIFI opensource package groups"
LICENSE = "BSD-3-Clause"
PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = ' \
    packagegroup-qti-wifi \
    \
    ${@bb.utils.contains("COMBINED_FEATURES", "qti-wifi", bb.utils.contains("MACHINE_FEATURES", "naples", "packagegroup-qti-wifi-naples", "", d), "", d)} \
    ${@bb.utils.contains("COMBINED_FEATURES", "qti-wifi", bb.utils.contains("MACHINE_FEATURES", "qti-cherokee", "packagegroup-qti-wifi-cherokee", "", d), "", d)} \
    ${@bb.utils.contains("COMBINED_FEATURES", "qti-wifi", "packagegroup-qti-wifi-tools", "", d)} \
    '

RDEPENDS_packagegroup-qti-wifi = ' \
    ${@bb.utils.contains("COMBINED_FEATURES", "qti-wifi", bb.utils.contains("MACHINE_FEATURES", "naples", "packagegroup-qti-wifi-naples", "", d), "", d)} \
    ${@bb.utils.contains("COMBINED_FEATURES", "qti-wifi", bb.utils.contains("MACHINE_FEATURES", "qti-cherokee", "packagegroup-qti-wifi-cherokee", "", d), "", d)} \
    ${@bb.utils.contains("COMBINED_FEATURES", "qti-wifi", "packagegroup-qti-wifi-tools", "", d)} \
    '

RDEPENDS_packagegroup-qti-wifi-naples = " \
    qcacld-hl \
    wpa-supplicant-qcacld-naples \
    hostap-daemon-qcacld \
    "

RDEPENDS_packagegroup-qti-wifi-cherokee = " \
    qcacld32-ll \
    wpa-supplicant-qcacld-cherokee \
    hostap-daemon-qcacld \
    qcacld32-ll-nf-debug \
    wpa-supplicant-8-lib \
    "

RDEPENDS_packagegroup-qti-wifi-tools = " \
    iw \
    wlan-conf \
    wireless-tools \
    "

RRECOMMENDS_packagegroup-qti-wifi-tools = " \
    sigma-dut \
    "
