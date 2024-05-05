inherit update-rc.d
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
REQUIRED_DISTRO_FEATURES = ""
SRC_URI += "\
    file://bind-files.sh \
    file://unbind-files.sh \
    file://start_robind \
    file://mount-copybind \
    file://umount-copybind \
    file://volatile-binds.service.in \
"
do_install() {
    install -d ${D}${base_sbindir}
    install -m 0755 mount-copybind ${D}${base_sbindir}/
if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
    install -d ${D}${systemd_unitdir}/system
    for service in ${SYSTEMD_SERVICE_${PN}}; do
        install -m 0644 $service ${D}${systemd_unitdir}/system/
        install -m 0755 umount-copybind ${D}${base_sbindir}/
    done
else
  install -m 0755 bind-files.sh ${D}${base_sbindir}/
  install -m 0755 unbind-files.sh ${D}${base_sbindir}/
  install -m 0755 start_robind -D ${D}${sysconfdir}/init.d/robind
fi
}

VOLATILE_BINDS = "\
/systemrw/adb_devid  /etc/adb_devid\n\
/systemrw/build.prop /etc/build.prop\n\
/systemrw/data /etc/data/\n\
/systemrw/data/adpl /etc/data/adpl/\n\
/systemrw/data/usb /etc/data/usb/\n\
/systemrw/data/miniupnpd /etc/data/miniupnpd/\n\
/systemrw/data/ipa /etc/data/ipa/\n\
/systemrw/rt_tables /etc/data/iproute2/rt_tables\n\
/systemrw/boot_hsusb_comp /etc/usb/boot_hsusb_comp\n\
/systemrw/boot_hsic_comp /etc/usb/boot_hsic_comp\n\
/systemrw/misc/wifi /etc/misc/wifi/\n\
/systemrw/bluetooth /etc/bluetooth/\n\
/systemrw/allplay /etc/allplay/\n\
/systemrw/adk-database /usr/share/adk-database/\n\
/systemrw/etc /etc/c2c/\n\
"
VOLATILE_BINDS_append_apq8017 = "\
/systemrw/AlexaClientSDKConfig.json  /etc/AlexaClientSDKConfig.json \n\
"
VOLATILE_BINDS_append_apq8009 = "\
/systemrw/alexa /etc/alexa/ \n\
/systemrw/var/lib/pulse /var/lib/pulse/ \n\
"
VOLATILE_BINDS_append_qcs40x = "\
/systemrw/var/lib/pulse /var/lib/pulse/ \n\
"
VOLATILE_BINDS_append_sdmsteppe = "\
/systemrw/var/lib/pulse /var/lib/pulse/ \n\
"

INITSCRIPT_PACKAGES =+ "${PN}"
INITSCRIPT_NAME_${PN} = "robind"
INITSCRIPT_PARAMS_${PN} = "start 37 S 2 3 4 5 ."
INITSCRIPT_PARAMS_${PN}_mdm = "start 30 S ."
