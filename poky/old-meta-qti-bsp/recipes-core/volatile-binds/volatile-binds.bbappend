inherit update-rc.d
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
REQUIRED_DISTRO_FEATURES = ""
SRC_URI += "\
    file://bind-files.sh \
    file://unbind-files.sh \
    file://start_robind \
"
do_install() {
    install -d ${D}${base_sbindir}
    install -m 0755 mount-copybind ${D}${base_sbindir}/
if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
    install -d ${D}${systemd_unitdir}/system
    for service in ${SYSTEMD_SERVICE_volatile-binds}; do
        install -m 0644 $service ${D}${systemd_unitdir}/system/
    done
else
  install -m 0755 bind-files.sh ${D}${base_sbindir}/
  install -m 0755 unbind-files.sh ${D}${base_sbindir}/
  install -m 0755 start_robind -D ${D}${sysconfdir}/init.d/robind
fi
}
VOLATILE_BINDS = "\
/persist/bootmisc.sh /etc/init.d/bootmisc.sh\n\
/var/volatile/lib /var/lib\n\
"
INITSCRIPT_PACKAGES =+ "${PN}"
INITSCRIPT_NAME_${PN} = "robind"
INITSCRIPT_PARAMS_${PN} = "start 91 2 3 4 5 ."
