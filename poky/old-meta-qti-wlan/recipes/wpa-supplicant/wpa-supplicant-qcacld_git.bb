include wpa-supplicant.inc

PR = "${INC_PR}.2"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://external/wpa_supplicant_8/"
SRC_URI += "file://defconfig-qcacld"
SRC_URI += "file://0001-fix-service-path.patch"

DEPENDS += "qmi"
DEPENDS += "qmi-framework"
FILES_${PN} += "/usr/include/* ${datadir}/dbus-1/system-services/*"
FILES_${PN} += "${systemd_unitdir}/system/*.service"

S = "${WORKDIR}/external/wpa_supplicant_8/wpa_supplicant"

do_configure() {
    install -m 0644 ${WORKDIR}/defconfig-qcacld .config
    echo "CFLAGS +=\"-I${STAGING_INCDIR}/libnl3\"" >> .config
}

do_install_append () {
    install -d ${D}/${sysconfdir}/dbus-1/system.d
    install -m 644 ${S}/dbus/dbus-wpa_supplicant.conf ${D}/${sysconfdir}/dbus-1/system.d
    install -d ${D}/${datadir}/dbus-1/system-services
    install -m 644 ${S}/dbus/*.service ${D}/${datadir}/dbus-1/system-services
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}/${systemd_unitdir}/system
        install -m 644 ${S}/systemd/*.service ${D}/${systemd_unitdir}/system
    fi
}
