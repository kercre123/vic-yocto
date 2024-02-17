SUMMARY = "Daemon to translate SELinux MCS/MLS sensitivity labels"
DESCRIPTION = "\
mcstrans provides an translation daemon to translate SELinux categories \
from internal representations to user defined representation."

SECTION = "base"
LICENSE = "GPLv2+"

SRC_URI += "file://mcstrans-de-bashify.patch \
            file://0001-mcstrans-fix-the-init-script.patch \
"

inherit systemd update-rc.d

DEPENDS += "libsepol libselinux libcap"

do_install_append() {
    install -d ${D}${sbindir}
    install -m 755 utils/untranscon ${D}${sbindir}/
    install -m 755 utils/transcon ${D}${sbindir}/

    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -d ${D}${sysconfdir}/tmpfiles.d
        echo "d ${localstatedir}/run/setrans - - - -" \
            > ${D}${sysconfdir}/tmpfiles.d/setrans.conf
    else
        install -d ${D}${sysconfdir}/default/volatiles
        echo "d root root 0755 /var/run/setrans none" \
            >${D}${sysconfdir}/default/volatiles/volatiles.80_mcstrans
    fi
    install -d ${D}${datadir}/mcstrans
    cp -r share/* ${D}${datadir}/mcstrans/.
}

SYSTEMD_SERVICE_mcstrans = "mcstrans.service"
INITSCRIPT_PACKAGES = "mcstrans"
INITSCRIPT_NAME_mcstrans = "mcstrans"
INITSCRIPT_PARAMS_mcstrans = "defaults"

pkg_postinst_mcstrans () {
    if [ -z "$D" ]; then
        if command -v systemd-tmpfiles >/dev/null; then
            systemd-tmpfiles --create ${sysconfdir}/tmpfiles.d/setrans.conf
        elif [ -e ${sysconfdir}/init.d/populate-volatile.sh ]; then
            ${sysconfdir}/init.d/populate-volatile.sh update
        fi
    fi
}
