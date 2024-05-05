# Packages to be included in rootfs creation.
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "coreutils"
IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libnl"
IMAGE_INSTALL += "libstdc++"
IMAGE_INSTALL += "libxml2"
IMAGE_INSTALL += "openssl"
IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "powerapp-reboot"
IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "start-scripts-misc-daemon"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-leprop"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-post-boot"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'systemd-machine-units', '', d)}"

IMAGE_EXT4_SELINUX_OPTIONS = "${@bb.utils.contains('DISTRO_FEATURES','selinux',' -S ${SELINUX_FILE_CONTEXTS} ','',d)}"

do_makesystem() {
    if ${@bb.utils.contains('DISTRO_FEATURES','nand-boot','false','true',d)}; then
     if [ -f ${THISDIR}/${BASEMACHINE}/${BASEMACHINE}-fsconfig.conf ]; then
         cp ${THISDIR}/${BASEMACHINE}/${BASEMACHINE}-fsconfig.conf ${WORKDIR}/rootfs-fsconfig.conf
         make_ext4fs -B ${DEPLOY_DIR_IMAGE}/system.map -C ${WORKDIR}/rootfs-fsconfig.conf -s -a / ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
     else
         make_ext4fs -B ${DEPLOY_DIR_IMAGE}/system.map -s -a / ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
     fi
     make_ext4fs -s -a /systemrw ${IMAGE_EXT4_SELINUX_OPTIONS} -b 4096 -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMRWIMAGE_TARGET}
    fi
}
