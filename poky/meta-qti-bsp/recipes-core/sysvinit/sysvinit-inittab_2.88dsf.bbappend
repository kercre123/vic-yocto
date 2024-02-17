FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI += "\
           file://${BASEMACHINE}/inittab \
"

USE_VT = "0"
SYSVINIT_ENABLED_GETTYS = ""

do_install() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/${BASEMACHINE}/inittab ${D}${sysconfdir}/inittab

    # SERIAL_CONSOLE is set from machine conf files in format 'baudrate terminal' (e.g. '115200 ttyMSM0')
    if [ ! -z "${SERIAL_CONSOLE}" ]; then
       tmp="${SERIAL_CONSOLE}"
       BAUDRATE=`echo $tmp | head -n1 | awk '{print $1;}'`
       TERMINAL=`echo $tmp | head -n1 | awk '{print $2;}'`
       echo "S:023456:respawn:${base_sbindir}/getty -L ${TERMINAL} ${BAUDRATE} console" >> ${D}${sysconfdir}/inittab
    fi

    if [ "${USE_VT}" = "1" ]; then
        cat <<EOF >>${D}${sysconfdir}/inittab
# ${base_sbindir}/getty invocations for the runlevels.
#
# The "id" field MUST be the same as the last
# characters of the device (after "tty").
#
# Format:
#  <id>:<runlevels>:<action>:<process>
#

EOF

        for n in ${SYSVINIT_ENABLED_GETTYS}
        do
            echo "$n:2345:respawn:${base_sbindir}/getty 38400 tty$n" >> ${D}${sysconfdir}/inittab
        done
        echo "" >> ${D}${sysconfdir}/inittab
    fi
}
