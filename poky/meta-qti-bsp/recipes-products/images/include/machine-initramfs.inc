DEPENDS += " \
             virtual/kernel \
"

# Cofigure the final rootfs of initramfs image
do_rootfs_append_sdxprairie() {
    bb.build.exec_func('configure_final_image',d)
}
# Replace the macro setting in initramfs init scripts for different project
do_macro_replace () {
    if ${@bb.utils.contains('DISTRO_FEATURES','nad-prod','true','false',d)}; then
        sed -i "s/UBI_PART_NAME=.*/UBI_PART_NAME=\"nad_ubi\"/g" ${IMAGE_ROOTFS}/init
    fi

}

IMAGE_PREPROCESS_COMMAND_append = " ${@bb.utils.contains('DISTRO_FEATURES', 'nad-prod', 'do_macro_replace;', '', d)} "
