# Rootfs creation.
IMAGE_INSTALL += "packagegroup-core-boot"
#IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# Target SDK Packages
TOOLCHAIN_TARGET_TASK_append_sa515m += "glib-2.0-dev"
TOOLCHAIN_TARGET_TASK_append_sa515m += "curl-dev"

# The MSM Linux minimal boot image files.
IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "libnl"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libstdc++"
IMAGE_INSTALL += "libxml2"
IMAGE_INSTALL += "openssl"
IMAGE_INSTALL += "openssl-bin"
IMAGE_INSTALL += "perl"
IMAGE_INSTALL += "powerapp"
IMAGE_INSTALL += "powerapp-powerconfig"
IMAGE_INSTALL += "powerapp-reboot"
IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "volatile-binds"
IMAGE_INSTALL += "edk2"
IMAGE_INSTALL += "coreutils"
IMAGE_INSTALL += "km-loader"
IMAGE_INSTALL += "systemd-machine-units"
# include abl image which support rootfs type as ubifs
# this image should be flashed while using non-squashfs file
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','nad-prod','abl-ubifs','',d)}"

# Install A/B recovery bins and its libs in system image if A/B boot is supported
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util recovery-ab-bin recovery-ab', '', d)}"

IMAGE_POSTPROCESS_COMMAND_append = "${@bb.utils.contains('IMAGE_FSTYPES', 'squashfs-xz', 'create_ext4_images;','',d)}"
IMAGE_CMD_ubi  = " "
IMAGE_CMD_ubifs  = " "

# Squashfs rootfs for flashless needs nbd mount points,
# so create a copy of rootfs for flashless to fixup.
create_squashfs_rootimg() {
  if ${@bb.utils.contains('IMAGE_FSTYPES', 'squashfs-xz', 'true', 'false', d)}; then
       mkdir -p ${IMAGE_ROOTFS_FLASHLESS}
       cp -R ${IMAGE_ROOTFS}/. ${IMAGE_ROOTFS_FLASHLESS}/.
       fixup_systemd_mounts_services_flashless_squashfs_rootfs
       mksquashfs ${IMAGE_ROOTFS_FLASHLESS} ${RAMDISK} -context-file ${SELINUX_FILE_CONTEXTS} -noappend -comp xz

   fi

}

fixup_systemd_mounts_services_flashless_squashfs_rootfs() {
# Mount remote squashfs partitions for flashless
 cp ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/data-nbd.mount \
           ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/data.mount
 cp ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/systemrw-nbd.mount \
           ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/systemrw.mount
 cp ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/persist-nbd.mount \
           ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/persist.mount
 cp ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/firmware-nbd-mount.service \
           ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/firmware-mount.service
 cp ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/usb_flashless.service \
           ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/usb.service
 ln -sf ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/nbdclient.service \
               ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/local-fs.target.wants/nbdclient.service
 rm -rf ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/multi-user.target.wants/cache.mount
 rm -rf ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/usb.service.d/usb_data_service.conf
 rm -rf ${IMAGE_ROOTFS_FLASHLESS}/lib/systemd/system/QCMAP_copybind.service.d/QCMAP_copybind.conf
}

create_ext4_images() {
   if ${@bb.utils.contains('DISTRO_FEATURES', 'flashless', 'true', 'false', d)}; then
          create_usr_ext4_img
          make_ext4fs -s -b 4096 -l ${PERSIST_SIZE_EXT4} -L ${PERSIST_LABEL} ${DEPLOY_DIR_IMAGE_EMMC}/${PERSISTIMAGE_TARGET} ${PERSIST_IMAGE_ROOTFS}
          chmod 644 ${OUTPUT_PERSIST_FILE}
          make_ext4fs -s -a /systemrw ${IMAGE_EXT4_SELINUX_OPTIONS} -b 4096 -l ${SYSTEMRW_SIZE_EXT4} -L ${SYSTEMRW_LABEL} ${DEPLOY_DIR_IMAGE_EMMC}/${SYSTEMRWIMAGE_TARGET}
   fi
}
