DEPENDS += "virtual/kernel"

DEPENDS += "pkgconfig-native gtk-doc-native gettext-native mkbootimg-native"
DEPENDS += "bzip2 fsconfig-native applypatch-native bsdiff-native ext4-utils-native mtd-utils-native"

DEPENDS += "${@bb.utils.contains('DISTRO_FEATURES', 'nand-squashfs', 'squashfs-tools-native', '', d)}"

# The OE-Core minimal boot image files.
IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

# The MSM Linux minimal boot image files.
IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "recovery-bin"
IMAGE_INSTALL += "recovery"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-recovery', '', d)}"
# IMAGE_INSTALL += "powerapp"
# IMAGE_INSTALL += "powerapp-powerconfig"
# IMAGE_INSTALL += "powerapp-reboot"
# IMAGE_INSTALL += "powerapp-shutdown"
IMAGE_INSTALL += "start-scripts-find-recovery-partitions"
IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "mtd-utils-ubifs"
IMAGE_INSTALL += "e2fsprogs-e2fsck"
IMAGE_INSTALL += "e2fsprogs-mke2fs"
IMAGE_INSTALL += "libcomerr"
IMAGE_INSTALL += "libss"
IMAGE_INSTALL += "libe2p"
IMAGE_INSTALL += "libext2fs"

# Needed for generating recovery.ubi ignored for ext4
MKUBIFS_ARGS = "${@bb.utils.contains('IMAGE_FSTYPES', 'ubi', '-m 4096 -e 253952 -c 200 -F', '', d)}"
UBI_VOLNAME  = "${@bb.utils.contains('IMAGE_FSTYPES', 'ubi', 'rootfs', '', d)}"
EXT4_IMAGET_TYPE  = "${@bb.utils.contains('IMAGE_FSTYPES', 'ext4', True, False, d)}"

# Install abctl and dependency in recovery bins
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'nad-prod', 'abctl', '', d)}"
DEPENDS += "${@bb.utils.contains('DISTRO_FEATURES', 'nad-prod', 'libgpt virtual/kernel glib-2.0', '', d)}"

do_fsconfig() {
 chmod go-r ${IMAGE_ROOTFS}/etc/passwd
 if ${@bb.utils.contains('DISTRO_FEATURES','systemd','false','true',d)}; then
  if [ "${DISTRO_NAME}" == "msm-user" ]; then
   if [ -e ${IMAGE_ROOTFS}/etc/init.d/sysfs.sh]; then
    sed -i '/mount -t debugfs/ d' ${IMAGE_ROOTFS}/etc/init.d/sysfs.sh
   fi
  fi
  chmod -R o-rwx ${IMAGE_ROOTFS}/etc/init.d/
 fi
}
DEPLOY_DIR_IMAGE_NAND ?= "${DEPLOY_DIR_IMAGE}"
DEPLOY_DIR_IMAGE_EMMC ?= "${DEPLOY_DIR_IMAGE}"
UNSPARSED_RECOVERYFS_DEST = "${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools/BOOTABLE_IMAGES"

# Create a BOOTABLE_IMAGES dir in this location,
# so as to maintain directory structure for zipping
do_makesystem[cleandirs] += "${UNSPARSED_RECOVERYFS_DEST}"

# Below is to generate sparse ext4 recovery image (OE by default supports raw ext4 images)
do_makesystem() {
	if ${@bb.utils.contains('IMAGE_FSTYPES', 'ext4', 'true', 'false', d)}; then
        if ${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'false', 'true', d)}; then
        # SELinux feature is enabled
        if [[ "${DISTRO_FEATURES}" =~ "selinux" ]]; then
            make_ext4fs -s ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${RECOVERYFS_SIZE_EXT4} ${DEPLOY_DIR_IMAGE_EMMC}/${RECOVERYIMAGE_TARGET} ${IMAGE_ROOTFS}

            # Create an unsparse image as well to be included as part of ota target-files
            simg2img ${DEPLOY_DIR_IMAGE_EMMC}/${RECOVERYIMAGE_TARGET} ${UNSPARSED_RECOVERYFS_DEST}/recovery-unsparsed.ext4
        else
            make_ext4fs -s -l ${RECOVERYFS_SIZE_EXT4} ${DEPLOY_DIR_IMAGE_EMMC}/${RECOVERYIMAGE_TARGET} ${IMAGE_ROOTFS}

            # Create an unsparse image as well to be included as part of ota target-files
            simg2img ${DEPLOY_DIR_IMAGE_EMMC}/${RECOVERYIMAGE_TARGET} ${UNSPARSED_RECOVERYFS_DEST}/recovery-unsparsed.ext4
        fi
        fi
	fi
}

# The default ubi image generated doesn't have provision for selinux
# Hence, create a recoveryfs.ubi image which is selinux enabled
# Do this only when both selinux & nand-boot is enabled
IMAGE_PREPROCESS_COMMAND_append = " ${@bb.utils.contains('DISTRO_FEATURES', 'selinux nand-boot', 'create_recoveryfs_ubi_img_with_selinux;', '', d)} "

RECOVERYFS_SELINUX_CFG = "${DEPLOY_DIR_IMAGE_NAND}/ubinize-recoveryfs-selinux.cfg"
OUTPUT_UBIFS_IMAGE = "${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-selinux-${MACHINE}.ubifs"
OUTPUT_UBI_IMAGE = "${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-selinux-${MACHINE}.ubi"

create_recoveryfs_ubi_img_with_selinux() {
    # create ubinize cfg
    echo \[ubifs\] > "${RECOVERYFS_SELINUX_CFG}"
    echo mode=ubi >> "${RECOVERYFS_SELINUX_CFG}"
    echo image="${OUTPUT_UBIFS_IMAGE}" >> "${RECOVERYFS_SELINUX_CFG}"
    echo vol_id=0 >> "${RECOVERYFS_SELINUX_CFG}"
    echo vol_type=dynamic >> "${RECOVERYFS_SELINUX_CFG}"
    echo vol_name=${UBI_VOLNAME} >> "${RECOVERYFS_SELINUX_CFG}"
    echo vol_flags=autoresize >> "${RECOVERYFS_SELINUX_CFG}"

    # create ubifs image (with selinux option passed) and then ubi image
    mkfs.ubifs -r ${IMAGE_ROOTFS} --selinux=${SELINUX_FILE_CONTEXTS} -o ${OUTPUT_UBIFS_IMAGE} ${MKUBIFS_ARGS}
    ubinize -o ${OUTPUT_UBI_IMAGE} ${UBINIZE_ARGS} ${RECOVERYFS_SELINUX_CFG}
    chmod 644 ${OUTPUT_UBI_IMAGE}
}

do_rename_recovery_ubi_image() {
    # if nand-squahfs distro is enabled then rename ubifs file as ${MACHINE}-recoveryfs-ubifs.ubi
    #  since ${MACHINE}-recoveryfs.ubi is used for squshfs recovery image
    if ${@bb.utils.contains('DISTRO_FEATURES','nand-squashfs','true','false',d)}; then
        # rename machine-recovery-image.ubi if it exists
        if [ -f "${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-${MACHINE}.ubi" ]; then
            mv ${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-${MACHINE}.ubi ${RECOVERYFS_UBIFS_UBI_IMAGE}
        fi

        # if there is "selinux enabled" version of recoveryfs image,
        # we would like to use that instead
        if [ -f "${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-selinux-${MACHINE}.ubi" ]; then
            mv ${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-selinux-${MACHINE}.ubi ${RECOVERYFS_UBIFS_UBI_IMAGE}
        fi
    else
        #rename machine-recovery-image.ubi if it exists
        if [ -f "${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-${MACHINE}.ubi" ]; then
            mv ${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-${MACHINE}.ubi ${DEPLOY_DIR_IMAGE_NAND}/${MACHINE}-recoveryfs.ubi
        fi

        # if there is "selinux enabled" version of recoveryfs image,
        # we would like to use that instead
        if [ -f "${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-selinux-${MACHINE}.ubi" ]; then
            mv ${DEPLOY_DIR_IMAGE_NAND}/machine-recovery-image-selinux-${MACHINE}.ubi ${DEPLOY_DIR_IMAGE_NAND}/${MACHINE}-recoveryfs.ubi
        fi
    fi
}

# machine-recovery-image and machine-image both use base-files package.
# base-files mounts a couple of partitions during bootup.
# This is fine for mission mode but not recovery mode.
# We strip out these mount operations from machine-recovery-image.
remove_mount_operations() {
    # do this only for systemd-boot, since the mount operations
    # are being done only for systemd as of now.
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        # userdata
        if [ -L ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/data.mount ]; then
            rm ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/data.mount
        fi
        # cache
        if [ -L ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/cache.mount ]; then
            rm ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/cache.mount
        fi
        # persist
        if [ -L ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/persist.mount ]; then
            rm ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/persist.mount
        fi
        # modem on A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware-mount.service ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware-mount.service
        fi
        # modem on nand + non A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware.mount
        fi
        # dsp on nand + non A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp.mount
        fi
        # dsp on A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp-mount.service ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp-mount.service
        fi
        # bluetooth on nand + non A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth.mount
        fi
        # bluetooth on A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth-mount.service ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth-mount.service
        fi
        # systemrw
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/systemrw.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/systemrw.mount
        fi
    fi
}

# Create a recoveryfs image with squashfs in it.
# This squashfs image will be one of the volumes on a ubi device
IMAGE_PREPROCESS_COMMAND_append = " ${@bb.utils.contains('DISTRO_FEATURES', 'nand-squashfs', 'create_recoveryfs_squashfs_ubi_img', '', d)} "

RECOVERYFS_SQUASHFS_IMAGE ?= "${DEPLOY_DIR_IMAGE_NAND}/${BASEMACHINE}-recoveryfs.squash"
RECOVERYFS_SQUASHFS_UBI_IMAGE ?= "${DEPLOY_DIR_IMAGE_NAND}/${BASEMACHINE}-squashfs-recoveryfs.ubi"
UBINIZE_RECOVERYFS_SQUASHFS_CFG = "${DEPLOY_DIR_IMAGE_NAND}/recoveryfs_squashfs_ubinize.cfg"

create_recoveryfs_squashfs_ubi_img() {
    create_recoveryfs_squashfs_ubinize_config
    if ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'true', 'false', d)}; then
        mksquashfs ${IMAGE_ROOTFS} ${RECOVERYFS_SQUASHFS_IMAGE} -context-file ${SELINUX_FILE_CONTEXTS} -noappend -comp xz
    else
        mksquashfs ${IMAGE_ROOTFS} ${RECOVERYFS_SQUASHFS_IMAGE} -noappend -comp xz
    fi
    ubinize -o ${RECOVERYFS_SQUASHFS_UBI_IMAGE} ${UBINIZE_ARGS} ${UBINIZE_RECOVERYFS_SQUASHFS_CFG}
    chmod 644 ${RECOVERYFS_SQUASHFS_UBI_IMAGE}
}

create_recoveryfs_squashfs_ubinize_config() {
    echo \[recoveryfs_volume\] > "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
    echo mode=ubi >> "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
    echo image="${RECOVERYFS_SQUASHFS_IMAGE}" >> "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
    echo vol_id=0 >> "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
    echo vol_type=dynamic >> "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
    echo vol_name=recoveryfs >> "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
    echo vol_flags=autoresize >> "${UBINIZE_RECOVERYFS_SQUASHFS_CFG}"
}

# machine-recovery-image and machine-image both use base-files package.
# base-files mounts a couple of partitions during bootup.
# This is fine for mission mode but not recovery mode.
# We strip out these mount operations from machine-recovery-image.
remove_mount_operations() {
    # do this only for systemd-boot, since the mount operations
    # are being done only for systemd as of now.
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        # userdata
        if [ -L ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/data.mount ]; then
            rm ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/data.mount
        fi
        # cache
        if [ -L ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/cache.mount ]; then
            rm ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/cache.mount
        fi
        # persist
        if [ -L ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/persist.mount ]; then
            rm ${IMAGE_ROOTFS}/lib/systemd/system/sysinit.target.wants/persist.mount
        fi
        # modem on A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware-mount.service ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware-mount.service
        fi
        # modem on nand + non A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/firmware.mount
        fi
        # dsp on nand + non A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp.mount
        fi
        # dsp on A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp-mount.service ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/dsp-mount.service
        fi
        # bluetooth on nand + non A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth.mount
        fi
        # bluetooth on A/B
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth-mount.service ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/bluetooth-mount.service
        fi
        # systemrw
        if [ -L ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/systemrw.mount ]; then
            rm ${IMAGE_ROOTFS}/etc/systemd/system/local-fs.target.requires/systemrw.mount
        fi
    fi
    # copy recovery bins and libs in correct path
    cp -rp  ${IMAGE_ROOTFS}/recovery/* ${IMAGE_ROOTFS}/
    rm -rf ${IMAGE_ROOTFS}/recovery/
}

ROOTFS_POSTPROCESS_COMMAND += " do_fsconfig; remove_mount_operations; "

# Call function makesystem to generate sparse ext4 image
addtask makesystem after do_rootfs before do_build
do_makesystem[noexec] = "1"
do_make_veritybootimg[noexec] = "1"
# rename recovery images only after they are copied to DEPLOY_DIR_NAND
addtask rename_recovery_ubi_image after do_image_complete before do_build
