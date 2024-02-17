DEPENDS_append_aarch64 += " lib32-lk"
DEPENDS_append_arm += " lk"

DEPENDS += "virtual/kernel"

DEPENDS += "pkgconfig-native gtk-doc-native gettext-native mkbootimg-native dtbtool-native"
DEPENDS += "bzip2 fsconfig-native applypatch-native bsdiff-native ext4-utils-native"

# The OE-Core minimal boot image files.
IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# The MSM Linux minimal boot image files.
IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "recovery"
IMAGE_INSTALL += "powerapp"
IMAGE_INSTALL += "powerapp-powerconfig"
IMAGE_INSTALL += "powerapp-reboot"
IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "start-scripts-find-recovery-partitions"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "mtd-utils-ubifs"
IMAGE_INSTALL += "e2fsprogs-e2fsck"
IMAGE_INSTALL += "e2fsprogs-mke2fs"
IMAGE_INSTALL += "libcomerr"
IMAGE_INSTALL += "libss"
IMAGE_INSTALL += "libe2p"
IMAGE_INSTALL += "libext2fs"

# utils and scripts needed for SELinux
IMAGE_INSTALL += "${@base_contains('DISTRO_FEATURES','selinux','packagegroup-core-selinux',' ',d)}"
IMAGE_INSTALL += "${@base_contains('DISTRO_FEATURES','selinux','selinux-config',' ',d)}"
IMAGE_INSTALL += "${@base_contains('DISTRO_FEATURES','selinux','refpolicy-mls',' ',d)}"

# Limit the recovery image size to 136M
RECOVERYFS_SIZE_EXT4 ?= "136000000"

# Needed for generating recovery.ubi ignored for ext4
MKUBIFS_ARGS = "${@bb.utils.contains('IMAGE_FSTYPES', 'ubi', '-m 4096 -e 253952 -c 83 -F', '', d)}"
UBI_VOLNAME  = "${@bb.utils.contains('IMAGE_FSTYPES', 'ubi', 'rootfs', '', d)}"

SELINUX_FILE_CONTEXTS = "${IMAGE_ROOTFS}/etc/selinux/mls/contexts/files/file_contexts"

# Below is to generate sparse ext4 recovery image (OE by default supports raw ext4 images)
do_makesystem() {
    if [ "${MACHINE}" != "mdm9607" ]; then
        # SELinux feature is enabled
        if [[ "${DISTRO_FEATURES}" =~ "selinux" ]]; then
            make_ext4fs -s -a / -S ${SELINUX_FILE_CONTEXTS} -l ${RECOVERYFS_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${MACHINE}-recovery.ext4 ${IMAGE_ROOTFS}
        else
            make_ext4fs -s -l ${RECOVERYFS_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${MACHINE}-recovery.ext4 ${IMAGE_ROOTFS}
        fi
    fi

    #rename machine-recovery-image.ubi if exists
    if [ -f "${DEPLOY_DIR_IMAGE}/machine-recovery-image-${MACHINE}.ubi" ]; then
        mv ${DEPLOY_DIR_IMAGE}/machine-recovery-image-${MACHINE}.ubi ${DEPLOY_DIR_IMAGE}/${MACHINE}-recovery.ubi
    fi
}

# Call function makesystem to generate sparse ext4 image
addtask makesystem after do_rootfs before do_build
