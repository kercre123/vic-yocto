PR .= ".3"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://volatiles.04_bind"

do_install_append() {
	install -d ${D}${sysconfdir}/default/volatiles
	install -m 0644 ${WORKDIR}/volatiles.04_bind ${D}${sysconfdir}/default/volatiles/volatiles.04_bind

	sed -i '/^\s*\/usr\/sbin\/rndc-confgen/a\
	    [ -x /sbin/restorecon ] && /sbin/restorecon -F /etc/bind/rndc.key' ${D}${sysconfdir}/init.d/bind
}
