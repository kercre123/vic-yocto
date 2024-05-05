include compat-wireless.inc

do_unpack[deptask] = "do_populate_sysroot"
PR = "${INC_PR}.5"

SRC_URI += "file://ath6kl_ctrl_wlan"

do_configure() {
    ./scripts/driver-select ath6kl
}

do_install_append() {
    install -m 0755 ${WORKDIR}/ath6kl_ctrl_wlan -D ${D}${sysconfdir}/init.d/wlan
    install -m 0644 drivers/net/wireless/ath/ath6kl/wmi.h -D ${D}${includedir}/ath6kl/wmi.h
    echo "device=AR6004" > ${B}/wlan_config
    install -m 0644 ${B}/wlan_config -D ${D}${sysconfdir}/wlan_config
}
