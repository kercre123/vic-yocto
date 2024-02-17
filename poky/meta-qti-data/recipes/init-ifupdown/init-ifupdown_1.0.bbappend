# Runlevels for networking daemon in mdm targets
pkg_postinst_${PN}_mdm () {
        [ -n "$D" ] && OPT="-r $D" || OPT="-s"
        if [ "${MACHINE}" == "sdxpoorwills" ]; then
          update-rc.d $OPT -f networking remove
          update-rc.d $OPT networking start 30 S . stop 80 0 6 1 .
        fi
}

