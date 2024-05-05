inherit ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'enable-audit', '', d)}

do_install_append() {
	if ${@bb.utils.contains('PACKAGECONFIG', 'backlight', 'true', 'false', d)}; then
		install -d ${D}${localstatedir}/lib/systemd/backlight
	fi
}
