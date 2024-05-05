require connman.inc

SRC_URI  = "${KERNELORG_MIRROR}/linux/network/${BPN}/${BP}.tar.xz \
            file://0001-plugin.h-Change-visibility-to-default-for-debug-symb.patch \
            file://0001-connman.service-stop-systemd-resolved-when-we-use-co.patch \
            file://connman \
            file://connman-main.conf \
            file://no-version-scripts.patch \
            file://1000-Skip-calling-xtables_insmod.patch \
            file://1001-Add-conf-settings-for-WiFi-tuning-params.patch \
            file://1002-Disable-NTP-by-default.patch \
            file://1003-Allow-the-net-user-to-access-connman-over-DBus.patch \
            "
SRC_URI_append_libc-musl = " file://0002-resolve-musl-does-not-implement-res_ninit.patch \
                             "

EXTRA_OECONF += "--localstatedir=/data"

SRC_URI[md5sum] = "dae77d9c904d2c223ae849e32079d57e"
SRC_URI[sha256sum] = "c789db41cc443fa41e661217ea321492ad59a004bebcd1aa013f3bc10a6e0074"

RRECOMMENDS_${PN} = "connman-conf connman-wait-online"
