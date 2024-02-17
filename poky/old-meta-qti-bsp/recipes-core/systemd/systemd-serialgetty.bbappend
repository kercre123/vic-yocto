FILESEXTRAPATHS_prepend := "${THISDIR}/systemd-serialgetty:"

SRC_URI += "file://serial-getty@ttyHSL0.service"

SERIAL_CONSOLE = "115200 ttyHSL0"

# Add to default do_install
do_install_prepend() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'qti-perf', 'false', 'true', d)}; then
	install -d ${D}${systemd_unitdir}/system/
	install -d ${D}${sysconfdir}/systemd/system/getty.target.wants/
	install -m 0644 ${WORKDIR}/serial-getty@ttyHSL0.service \
		${D}${systemd_unitdir}/system/serial-getty@.service
	# enable the service
	ln -sf ${systemd_unitdir}/system/serial-getty@.service \
		${D}${sysconfdir}/systemd/system/getty.target.wants/serial-getty@ttyHSL0.service
    fi
}
