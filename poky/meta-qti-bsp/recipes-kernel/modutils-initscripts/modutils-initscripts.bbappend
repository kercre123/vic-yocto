updatercd_postinst() {
        [ -n "$D" ] && OPT="-r $D" || OPT="-s"
        update-rc.d $OPT -f modutils.sh remove
}
