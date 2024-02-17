# The MSM Linux minimal image rootfs creation files.

# lk supports only 32-bit compilation.
DEPENDS_append_arm += " lk"
DEPENDS_append_aarch64 += " lib32-lk"

# Compute sha1sum of unsparsed recovery&recoveryfs images
# only after recoveryfs.ext4 image creation has already happened.
do_rootfs[depends] += "machine-recovery-image:do_makesystem"
# Formats of root filesystem images.
IMAGE_FSTYPES += "ext4"

IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','selinux', 'packagegroup-core-selinux', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','selinux', 'selinux-config', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','selinux', 'refpolicy-mls', '', d)}"

IMAGE_INSTALL += "start-scripts-firmware-links"
IMAGE_INSTALL += "start-scripts-find-partitions"
IMAGE_INSTALL += "start-scripts-misc-daemon"
IMAGE_INSTALL += "start-scripts-update-perms"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util', '', d)}"
IMAGE_INSTALL += "start-scripts-recovery-updater"
IMAGE_INSTALL += "sec-config"

IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "dhcpcd"
IMAGE_INSTALL += "binder"

IMAGE_INSTALL += "glib-2.0"
IMAGE_INSTALL += "libcutils"
IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "libxml2"
IMAGE_INSTALL += "system-core-adbd"
IMAGE_INSTALL += "system-core-logd"
IMAGE_INSTALL += "system-core-leprop"
IMAGE_INSTALL += "system-core-usb"
IMAGE_INSTALL += "${@oe.utils.conditional('USER_BUILD','1','','system-core-debuggerd',d)}"
IMAGE_INSTALL += "system-core-post-boot"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "wireless-tools"

#32-bit only packages
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-libnl', 'libnl', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-gtest', 'gtest', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-cairo', 'cairo', d)}"

IMAGE_INSTALL += "libpagemap"

IMAGE_INSTALL += "libuv"
IMAGE_INSTALL += "libwebsockets"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)
SELINUX_FILE_CONTEXTS = "${IMAGE_ROOTFS}/etc/selinux/mls/contexts/files/file_contexts"

IMAGE_EXT4_SELINUX_OPTIONS = "${@bb.utils.contains('DISTRO_FEATURES','selinux',' -S ${SELINUX_FILE_CONTEXTS} ','',d)}"

do_fsconfig_append() {
if [ "${DISTRO_NAME}" == "msm-user" ]; then
sed -i '/mount -t debugfs/ d' ${IMAGE_ROOTFS}/etc/init.d/sysfs.sh
fi
    chmod -R o-rwx ${IMAGE_ROOTFS}/etc/init.d/
}

UNSPARSED_RECOVERYFS_DEST = "${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools/BOOTABLE_IMAGES"

# Compute sha1 of recovery(fs) images
# as an extension of the do_rootfs() task.
python do_rootfs_append() {
    import subprocess

    # Create a "recoveryupgrade" directory which will house the
    # data needed by the recovery/recoveryfs upgrade mechanism.
    cmd1 = "mkdir -p ${IMAGE_ROOTFS}/recoveryupgrade"
    subprocess.call(cmd1, shell=True)

    # first compute sha1 of boot.img, which is same as recovery partition's content
    cmd2 = "sha1sum ${DEPLOY_DIR_IMAGE}/${BOOTIMAGE_TARGET} | awk '{ print $1 }'" \
           " > ${IMAGE_ROOTFS}/recoveryupgrade/recoveryimg_sha1"
    subprocess.call(cmd2, shell=True)
    cmd3 = "stat --printf=\"%s\" ${DEPLOY_DIR_IMAGE}/${BOOTIMAGE_TARGET}" \
           " > ${IMAGE_ROOTFS}/recoveryupgrade/recoveryimg_length"
    subprocess.call(cmd3, shell=True)

    # now compute sha1 of the unsparsed recoveryfs image
    cmd4 = "sha1sum ${UNSPARSED_RECOVERYFS_DEST}/recovery-unsparsed.ext4 | " \
           " awk '{ print $1 }' > ${IMAGE_ROOTFS}/recoveryupgrade/recoveryfsimg_sha1"
    subprocess.call(cmd4, shell=True)
    cmd5 = "stat --printf=\"%s\" ${UNSPARSED_RECOVERYFS_DEST}/recovery-unsparsed.ext4" \
           " > ${IMAGE_ROOTFS}/recoveryupgrade/recoveryfsimg_length"
    subprocess.call(cmd5, shell=True)
}

do_makesystem() {
    cp ${THISDIR}/${BASEMACHINE}/apq8053-fsconfig.conf ${WORKDIR}/rootfs-fsconfig.conf
    make_ext4fs -C ${WORKDIR}/rootfs-fsconfig.conf ${IMAGE_EXT4_EXTRA_OPTIONS} -s -a / ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
    make_ext4fs  -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMRWIMAGE_TARGET}
}
