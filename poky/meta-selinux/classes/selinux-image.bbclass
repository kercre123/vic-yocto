selinux_set_labels () {
    POL_TYPE=$(sed -n -e "s&^SELINUXTYPE[[:space:]]*=[[:space:]]*\([0-9A-Za-z_]\+\)&\1&p" ${IMAGE_ROOTFS}/${sysconfdir}/selinux/config)
    if ! setfiles -m -r ${IMAGE_ROOTFS} ${IMAGE_ROOTFS}/${sysconfdir}/selinux/${POL_TYPE}/contexts/files/file_contexts ${IMAGE_ROOTFS}
    then
        echo WARNING: Unable to set filesystem context, setfiles / restorecon must be run on the live image.
        touch ${IMAGE_ROOTFS}/.autorelabel
        exit 0
    fi
}

DEPENDS += "policycoreutils-native"

IMAGE_PREPROCESS_COMMAND += "selinux_set_labels ;"

inherit core-image
