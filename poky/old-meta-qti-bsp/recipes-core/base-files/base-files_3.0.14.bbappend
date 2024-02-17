FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"
DEPENDS = "base-passwd"

SRC_URI_append += "file://fstab"
SRC_URI_append += "file://ro-fstab"
SRC_URI_append_apq8017 += "file://apq8017/ro-fstab"
SRC_URI_append_apq8053 += "file://apq8053/ro-fstab"
SRC_URI_append_mdm9607 += "file://mdm9607/ro-fstab"
SRC_URI_append_mdm9650 += "file://mdm9650/ro-fstab"
SRC_URI_append_sdx20 += "file://sdx20/ro-fstab"

SRC_URI_append_apq8017 += "file://apq8017/cache.mount"
SRC_URI_append_apq8017 += "file://apq8017/firmware.mount"
SRC_URI_append_apq8017 += "file://apq8017/dsp.mount"
SRC_URI_append_apq8017 += "file://apq8017/media-card.mount"
SRC_URI_append_apq8017 += "file://apq8017/media-ram.mount"
SRC_URI_append_apq8017 += "file://apq8017/persist.mount"
SRC_URI_append_apq8017 += "file://apq8017/proc-bus-usb.mount"
SRC_URI_append_apq8017 += "file://apq8017/var-volatile.mount"
SRC_URI_append_apq8017 += "file://apq8017/dash.mount"
SRC_URI_append_apq8009 += "file://apq8009/robot-fstab"
SRC_URI_append_apq8009 += "file://apq8009/robot-factory-fstab"
SRC_URI_append_apq8009 += "file://apq8009/ro-fstab"

dirs755 += "/media/cf /media/net /media/ram \
            /media/union /media/realroot /media/hdd \
            /media/mmc1"

dirs755_append_apq8053 +="/persist /cache /dsp "
#TODO Enabling systemd we need to add /firmware in dirs_755 list.
dirs755_append_apq8009 += "/firmware /persist /factory"
dirs755_append_apq8017 += "/firmware /persist /cache /dsp"

do_install_append(){
    install -m 755 -o diag -g diag -d ${D}/media
    install -m 755 -o diag -g diag -d ${D}/mnt/sdcard
    if ${@base_contains('DISTRO_FEATURES','ro-rootfs','true','false',d)}; then
        # Override fstab for apq8017
        if [ ${BASEMACHINE} == "apq8053" || ${BASEMACHINE} == "mdm9607" || ${BASEMACHINE} == "sdx20" || ${BASEMACHINE} == "mdm9650" ]; then
            install -m 0644 ${WORKDIR}/${BASEMACHINE}/ro-fstab ${D}${sysconfdir}/fstab
        elif [  ${BASEMACHINE} == "apq8009" ]; then
            if [ "${PRODUCT}" == "robot" ] || [ "${PRODUCT}" == "robot-rome" ]; then
                if [ "${FACTORY}" == "1" ]; then
                    install -m 0644 ${WORKDIR}/${BASEMACHINE}/robot-factory-fstab ${D}${sysconfdir}/fstab
                else
                    install -m 0644 ${WORKDIR}/${BASEMACHINE}/robot-fstab ${D}${sysconfdir}/fstab
                fi
            else
                install -m 0644 ${WORKDIR}/${BASEMACHINE}/ro-fstab ${D}${sysconfdir}/fstab
            fi
        elif [ ${BASEMACHINE} == "apq8017" ]; then
            install -m 0644 ${WORKDIR}/${BASEMACHINE}/ro-fstab ${D}${sysconfdir}/fstab
            install -d 0644 ${D}${sysconfdir}/systemd/system
            install -m 0644 ${WORKDIR}/apq8017/cache.mount ${D}${sysconfdir}/systemd/system/cache.mount
            install -m 0644 ${WORKDIR}/apq8017/firmware.mount ${D}${sysconfdir}/systemd/system/firmware.mount
            install -m 0644 ${WORKDIR}/apq8017/dsp.mount ${D}${sysconfdir}/systemd/system/dsp.mount
            install -m 0644 ${WORKDIR}/apq8017/media-card.mount ${D}${sysconfdir}/systemd/system/media-card.mount
            install -m 0644 ${WORKDIR}/apq8017/media-ram.mount ${D}${sysconfdir}/systemd/system/media-ram.mount
            install -m 0644 ${WORKDIR}/apq8017/persist.mount ${D}${sysconfdir}/systemd/system/persist.mount
            install -m 0644 ${WORKDIR}/apq8017/proc-bus-usb.mount ${D}${sysconfdir}/systemd/system/proc-bus-usb.mount
            install -m 0644 ${WORKDIR}/apq8017/var-volatile.mount ${D}${sysconfdir}/systemd/system/var-volatile.mount
            install -m 0644 ${WORKDIR}/apq8017/dash.mount ${D}${sysconfdir}/systemd/system/-.mount
            install -d 0644 ${D}${sysconfdir}/systemd/system/local-fs.target.requires
            ln -sf  ../cache.mount  ${D}${sysconfdir}/systemd/system/local-fs.target.requires/cache.mount
            ln -sf  ../firmware.mount  ${D}${sysconfdir}/systemd/system/local-fs.target.requires/firmware.mount
            ln -sf  ../dsp.mount  ${D}${sysconfdir}/systemd/system/local-fs.target.requires/dsp.mount
            ln -sf  ../persist.mount  ${D}${sysconfdir}/systemd/system/local-fs.target.requires/persist.mount
            ln -sf  ../cache.mount  ${D}${sysconfdir}/systemd/system/local-fs.target.requires/var-volatile.mount
            ln -sf  ../-.mount  ${D}${sysconfdir}/systemd/system/local-fs.target.requires/-.mount
        else
            install -m 0644 ${WORKDIR}/ro-fstab ${D}${sysconfdir}/fstab
        fi
    else
        install -m 0644 ${WORKDIR}/fstab ${D}${sysconfdir}/fstab
    fi
    ln -s /mnt/sdcard ${D}/sdcard
    rmdir ${D}/tmp
    ln -s /var/tmp ${D}/tmp
    ln -s /var/run/resolv.conf ${D}/etc/resolv.conf
}
