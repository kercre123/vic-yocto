FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI += "\
           file://${BASEMACHINE}/inittab \
"

SERIAL_CONSOLE = "115200 console"
TERMINAL = "${@base_contains('BASEMACHINE', 'apq8098', 'ttyMSM0', 'ttyHSL0', d)}"
USE_VT = "0"
SYSVINIT_ENABLED_GETTYS = ""

do_install() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/${BASEMACHINE}/inittab ${D}${sysconfdir}/inittab
    if [ ! -z "${SERIAL_CONSOLE}" ]; then
        echo "S:023456:respawn:${base_sbindir}/getty -L ${TERMINAL} ${SERIAL_CONSOLE}" >> ${D}${sysconfdir}/inittab
    fi

    idx=0
#   tmp="${SERIAL_CONSOLES}"
    tmp=""
    for i in $tmp
    do
        j=`echo ${i} | sed s/\;/\ /g`
        echo "${idx}:2345:respawn:${base_sbindir}/getty ${j}" >> ${D}${sysconfdir}/inittab
        idx=`expr $idx + 1`
    done

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
