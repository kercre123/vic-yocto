require conf/distro/qpermissions.conf

do_update_files() {
    set +e
    export FILE_PERMISSIONS="${QPERM_FILE}"
    if [ "$FILE_PERMISSIONS" != "" ] ; then
        for each_file in ${FILE_PERMISSIONS};    do
            path="$(cut -d ":" -f 1 <<< $each_file)"
            user="$(cut -d ":" -f 2 <<< $each_file)"
            group="$(cut -d ":" -f 3 <<< $each_file)"
            chown -R $user:$group ${D}$path
        done
    fi
}

do_package[prefuncs] += "do_update_files"
