require bluez5.inc

PR = "r1"

DEPENDS += "readline"

SRC_URI[md5sum] = "2a575ec06aeaeadca9605b2f8173e00a"
SRC_URI[sha256sum] = "92bf4ce87d58014794ef6b22dc0a13b0b19acdf9c96870391c935d1e01a43ffa"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"

CFLAGS_append = " -D_PLATFORM_MDM_"

do_install_append() {
        install -d ${D}${sysconfdir}/bluetooth/

        if [ -f ${S}/profiles/network/network.conf ]; then
            install -m 0644 ${S}/profiles/network/network.conf ${D}/${sysconfdir}/bluetooth/
        fi

        if [ -f ${S}/profiles/input/input.conf ]; then
            install -m 0644 ${S}/profiles/input/input.conf ${D}/${sysconfdir}/bluetooth/
        fi
        # at_console doesn't really work with the current state of OE, so punch some more holes so people can actually use BT
        install -m 0644 ${S}/src/bluetooth.conf ${D}/${sysconfdir}/dbus-1/system.d/
}

PACKAGES =+ "libasound-module-bluez"

FILES_libasound-module-bluez = "\
  ${libdir}/alsa-lib/libasound_module_ctl_bluetooth.so \
  ${libdir}/alsa-lib/libasound_module_pcm_bluetooth.so \
  ${datadir}/alsa\
"
FILES_${PN} += "\
  ${base_libdir}/udev/ \
  ${base_libdir}/systemd/ \
"
FILES_${PN}-dev += "\
  ${libdir}/alsa-lib/libasound_module_ctl_bluetooth.la \
  ${libdir}/alsa-lib/libasound_module_pcm_bluetooth.la \
"

PACKAGES =+ "${PN}-test"
RDEPENDS_${PN}-test = "python"
FILES_${PN}-test += "\
  ${libdir}/bluez/ \
"
