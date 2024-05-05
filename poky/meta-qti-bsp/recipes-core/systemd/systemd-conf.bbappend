FILESEXTRAPATHS_prepend := "${THISDIR}/systemd:"

SRC_URI += "file://sysctl-coredump.conf"
SRC_URI += "file://limits-coredump.conf"

# Modify default CONFFILES as per machine needs

# coredump.conf
do_install_append() {
   if [ "${SYSTEMD_ENABLE_COREDUMP}" == "1" ]; then
       sed -i "s#@COREDUMP_PATH@#${SYSTEMD_COREDUMP_PATH}#" ${WORKDIR}/sysctl-coredump.conf

       install -m 0644 ${WORKDIR}/sysctl-coredump.conf -D ${D}${sysconfdir}/sysctl.d/sys-coredump.conf
       install -m 0644 ${WORKDIR}/limits-coredump.conf -D ${D}${sysconfdir}/security/limits.d/sys-coredump.conf

       #create coredump folder if needed
       install -m 0666 -d ${D}${SYSTEMD_COREDUMP_PATH}
   else
       rm -f ${D}${sysconfdir}/systemd/coredump.conf
   fi
}

FILES_${PN} += "${sysconfdir}/sysctl.d/* ${sysconfdir}/security/limits.d/* ${SYSTEMD_COREDUMP_PATH}"

# journald.conf
do_install_append() {
}

# logind.conf
do_install_append() {
    # Ignore PowerKey
    sed -i -e 's/#HandlePowerKey=poweroff/HandlePowerKey=ignore/' ${D}${sysconfdir}/systemd/logind.conf
}

# system.conf
do_install_append() {
}

# user.conf
do_install_append() {
}
