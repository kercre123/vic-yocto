#compile abl
DEPLOY_DIR_IMAGE_EMMC ?= "${DEPLOY_DIR_IMAGE}"
RDEPENDS  += "${@bb.utils.contains('TARGET_ARCH', 'arm','lib64-edk2','edk2',d)}"
RDEPENDS  += "${@bb.utils.contains('TARGET_ARCH', 'arm','lib64-abl-ubifs','abl-ubifs',d)}"

# Target SDK Packages
TOOLCHAIN_TARGET_TASK_append_sa2150p += "glib-2.0-dev"
TOOLCHAIN_TARGET_TASK_append_sa2150p += "curl-dev"

# Packages to be included in rootfs creation.
IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_INSTALL += "start-scripts-misc-daemon"
IMAGE_INSTALL += "chrony"

IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libxml2"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-leprop"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "system-core-post-boot"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "systemd-machine-units"
IMAGE_INSTALL += "screen"
# IMAGE_INSTALL += "wireless-tools"
IMAGE_INSTALL +="${@bb.utils.contains('DISTRO_FEATURES', 'nand-boot', 'mtd-utils-ubifs', '', d)}"

# The MSM Linux minimal boot image files.
#IMAGE_INSTALL += "gdbserver"
#IMAGE_INSTALL += "gdb"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "libnl"
IMAGE_INSTALL += "libstdc++"
IMAGE_INSTALL += "openssl"
#IMAGE_INSTALL += "perl"
# IMAGE_INSTALL += "powerapp"
# IMAGE_INSTALL += "powerapp-powerconfig"
# IMAGE_INSTALL += "powerapp-reboot"
# IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "coreutils"

IMAGE_INSTALL += "libgomp"
TOOLCHAIN_TARGET_TASK_append += " libgomp-dev libgomp-staticdev libstdc++-staticdev"

IMAGE_EXT4_SELINUX_OPTIONS = "${@bb.utils.contains('DISTRO_FEATURES','selinux',' -S ${SELINUX_FILE_CONTEXTS} ','',d)}"

IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util', '', d)}"
DEPENDS += "squashfs-tools-native"

# Install A/B recovery bins and its libs in system image if A/B boot is supported
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'recovery-ab-bin', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'recovery-ab', '', d)}"

#Dont use upstream fuctions for fs generation.
IMAGE_CMD_ext4 = " "
IMAGE_CMD_ubi  = " "
EMMC_ROOTFS    = "emmc-rootfs"
EMMC_ROOTFS_DIR = "${IMAGE_ROOTFS}/../${EMMC_ROOTFS}"

addtask do_generate_rootfs_fsconfig after do_rootfs before do_makesystem

do_generate_rootfs_fsconfig() {
    if ${@bb.utils.contains('IMAGE_FSTYPES', 'ext4', 'true', 'false', d)}; then
        cp ${THISDIR}/${BASEMACHINE}/qcs40x-fsconfig.conf ${WORKDIR}/rootfs-fsconfig.conf
    fi
}

do_makesystem() {
 if ${@bb.utils.contains('IMAGE_FSTYPES', 'ext4', 'true', 'false', d)}; then
     fixup_systemd_mounts_services_ext4_rootfs
#preserve rootfs to be used by recovery for emmc ota package generation.
     rm -rf ${EMMC_ROOTFS_DIR}
     mkdir -p ${EMMC_ROOTFS_DIR}
     cp -pr ${IMAGE_ROOTFS}/*  ${EMMC_ROOTFS_DIR}/
#generate ext4 fs.
     make_ext4fs -C ${WORKDIR}/rootfs-fsconfig.conf -B ${DEPLOY_DIR_IMAGE}/system.map -s -a / ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE_EMMC}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
     make_ext4fs -s -a /systemrw ${IMAGE_EXT4_SELINUX_OPTIONS} -b 4096 -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE_EMMC}/${SYSTEMRWIMAGE_TARGET}
 fi
     # Create rootfs tar file for debugging on target
     tar -cvzf ${DEPLOY_DIR_IMAGE}/${MACHINE}.rootfs.tar.gz -C ${IMAGE_ROOTFS} .
}

fixup_systemd_mounts_services_ext4_rootfs() {
#Mount ext4 partitions at bootup.
 if ${@bb.utils.contains('DISTRO_FEATURES','ab-boot-support','true','false',d)}; then
   if [ -e ${IMAGE_ROOTFS}/lib/systemd/system/bt-fw-ab-ext4-mount.service ]; then
     cp ${IMAGE_ROOTFS}/lib/systemd/system/bt-fw-ab-ext4-mount.service ${IMAGE_ROOTFS}/lib/systemd/system/bt_firmware-mount.service
   fi
   cp ${IMAGE_ROOTFS}/lib/systemd/system/fw-ab-ext4-mount.service ${IMAGE_ROOTFS}/lib/systemd/system/firmware-mount.service
   cp ${IMAGE_ROOTFS}/lib/systemd/system/dsp-ab-ext4-mount.service ${IMAGE_ROOTFS}/lib/systemd/system/dsp-mount.service
 else
   rm -rf ${IMAGE_ROOTFS}/lib/systemd/system/bt_firmware-mount.service
   rm -rf ${IMAGE_ROOTFS}/lib/systemd/system/firmware-mount.service
   rm -rf ${IMAGE_ROOTFS}/lib/systemd/system/dsp-mount.service
 fi
 cp ${IMAGE_ROOTFS}/lib/systemd/system/systemrw-ext4.mount ${IMAGE_ROOTFS}/lib/systemd/system/systemrw.mount
 cp ${IMAGE_ROOTFS}/lib/systemd/system/persist-ext4.mount ${IMAGE_ROOTFS}/lib/systemd/system/persist.mount
 cp ${IMAGE_ROOTFS}/lib/systemd/system/cache-ext4.mount ${IMAGE_ROOTFS}/lib/systemd/system/cache.mount
 cp ${IMAGE_ROOTFS}/lib/systemd/system/data-ext4.mount  ${IMAGE_ROOTFS}/lib/systemd/system/data.mount
 if ${@bb.utils.contains('DISTRO_FEATURES', 'full-disk-encryption', 'true', 'false', d)}; then
   rm -rf ${IMAGE_ROOTFS}/lib/systemd/system/data.mount
   rm -rf ${IMAGE_ROOTFS}/lib/systemd/system/data-ext4.mount
 fi
}
