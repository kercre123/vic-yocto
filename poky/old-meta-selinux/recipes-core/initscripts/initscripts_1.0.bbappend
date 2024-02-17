PR .= ".3"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

do_install_append () {
	cat <<-EOF >> ${D}${sysconfdir}/init.d/populate-volatile.sh
touch /var/log/lastlog
test ! -x /sbin/restorecon || /sbin/restorecon -RF /var/volatile/ /var/lib /run \
    /etc/resolv.conf /etc/adjtime
EOF
	sed -i '/mount -n -o remount,$rootmode/i\test ! -x /sbin/restorecon || /sbin/restorecon -RF /run' \
	    ${D}${sysconfdir}/init.d/checkroot.sh
}
