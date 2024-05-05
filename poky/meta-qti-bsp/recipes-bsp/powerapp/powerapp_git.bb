inherit autotools-brokensep externalsrc

DESCRIPTION = "Powerapp tools"
HOMEPAGE = "http://codeaurora.org/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

EXTERNALSRC = "${WORKSPACE}/system/core/powerapp/"

PACKAGES =+ "${PN}-reboot ${PN}-shutdown ${PN}-powerconfig"
FILES_${PN}-reboot = " ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', "${sysconfdir}/initscripts/reboot", "${sysconfdir}/init.d/reboot", d)} "
FILES_${PN}-shutdown = " ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', "${sysconfdir}/initscripts/shutdown", "${sysconfdir}/init.d/shutdown", d)} "
FILES_${PN}-powerconfig = " ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', "${sysconfdir}/initscripts/power_config", "${sysconfdir}/init.d/power_config", d)} "
FILES_${PN} += "/data/*"
FILES_${PN} += "/lib/systemd/*"

# TODO - add depedency on virtual/sh
PROVIDES =+ "${PN}-reboot ${PN}-shutdown ${PN}-powerconfig"

PR = "r9"

do_install() {
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
           install -m 0755 ${B}/powerapp -D ${D}/sbin/powerapp
           install -m 0755 ${S}/reboot -D ${D}${sysconfdir}/initscripts/reboot
           install -m 0755 ${S}/reboot-bootloader -D ${D}/sbin/reboot-bootloader
           install -m 0755 ${S}/reboot-recovery -D ${D}/sbin/reboot-recovery
           install -m 0755 ${S}/reboot-cookie -D ${D}${userfsdatadir}/reboot-cookie
           install -m 0755 ${S}/reset_reboot_cookie -D ${D}${sysconfdir}/initscripts/reset_reboot_cookie
           install -m 0755 ${S}/shutdown -D ${D}${sysconfdir}/initscripts/shutdown
           install -m 0755 ${S}/start_power_config -D ${D}${sysconfdir}/initscripts/power_config
           ln ${D}${base_sbindir}/powerapp ${D}${base_sbindir}/sys_reboot
           ln ${D}${base_sbindir}/powerapp ${D}${base_sbindir}/sys_shutdown
           install -m 0644 ${S}/reset_reboot_cookie.service -D ${D}${systemd_unitdir}/system/reset_reboot_cookie.service
           install -m 0644 ${S}/power_config.service -D ${D}${systemd_unitdir}/system/power_config.service
           install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
           ln -sf ${systemd_unitdir}/system/reset_reboot_cookie.service ${D}${systemd_unitdir}/system/multi-user.target.wants/reset_reboot_cookie.service
           ln -sf ${systemd_unitdir}/system/power_config.service ${D}${systemd_unitdir}/system/multi-user.target.wants/power_config.service
        else
           install -m 0755 ${B}/powerapp -D ${D}/sbin/powerapp
           install -m 0755 ${S}/reboot -D ${D}${sysconfdir}/init.d/reboot
           install -m 0755 ${S}/reboot-bootloader -D ${D}/sbin/reboot-bootloader
           install -m 0755 ${S}/reboot-recovery -D ${D}/sbin/reboot-recovery
           install -m 0755 ${S}/reboot-cookie -D ${D}${userfsdatadir}/reboot-cookie
           install -m 0755 ${S}/reset_reboot_cookie -D ${D}${sysconfdir}/init.d/reset_reboot_cookie
           install -m 0755 ${S}/shutdown -D ${D}${sysconfdir}/init.d/shutdown
           install -m 0755 ${S}/start_power_config -D ${D}${sysconfdir}/init.d/power_config
           ln ${D}${base_sbindir}/powerapp ${D}${base_sbindir}/sys_reboot
           ln ${D}${base_sbindir}/powerapp ${D}${base_sbindir}/sys_shutdown
        fi
}


pkg_postinst_${PN}-reboot () {
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
           [ -n "$D" ] && OPT="-r $D" || OPT="-s"
           update-rc.d $OPT -f reboot remove
           update-rc.d $OPT reboot start 99 6 .
	fi
}

pkg_postinst_${PN}-shutdown () {
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
           [ -n "$D" ] && OPT="-r $D" || OPT="-s"
           update-rc.d $OPT -f shutdown remove
           update-rc.d $OPT shutdown start 99 0 .
	fi
}

pkg_postinst_${PN}-powerconfig () {
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
           [ -n "$D" ] && OPT="-r $D" || OPT="-s"
           update-rc.d $OPT -f power_config remove
           update-rc.d $OPT power_config start 99 2 3 4 5 . stop 50 0 1 6 .
	fi
}

pkg_postinst_${PN} () {
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
           [ -n "$D" ] && OPT="-r $D" || OPT="-s"
           update-rc.d $OPT -f reset_reboot_cookie remove
           update-rc.d $OPT reset_reboot_cookie start 55 2 3 4 5 .
        fi
}
