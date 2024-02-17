include hostap-daemon.inc

PR = "${INC_PR}.3"

SRC_URI += "file://defconfig-ath6kl"

do_configure() {
    install -m 0644 ${WORKDIR}/defconfig-ath6kl .config
}

do_configure_append() {
    echo "CFLAGS += -I${WORKSPACE}/wlan/include" >> .config
    echo "CFLAGS += -I${WORKSPACE}/wlan/host/include" >> .config
    echo "CFLAGS += -I${WORKSPACE}/wlan/host/os/linux/include" >> .config
    echo "CFLAGS += -I${WORKSPACE}/wlan/host/wlan/include" >> .config
}
