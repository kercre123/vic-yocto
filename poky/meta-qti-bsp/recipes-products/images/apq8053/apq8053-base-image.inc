# The MSM Linux image rootfs creation files.

# lk supports only 32-bit compilation.
DEPENDS_append_arm += " lk"
DEPENDS_append_aarch64 += " lib32-lk"

# Compute sha1sum of unsparsed recovery&recoveryfs images
# only after recoveryfs.ext4 image creation has already happened.
do_rootfs[depends] += "${@["","machine-recovery-image:do_makesystem"][(d.getVar('RECOVERY_IMAGE', True) == '1')]}"

IMAGE_INSTALL += "packagegroup-core-boot"
#IMAGE_INSTALL += "${ROOTFS_PKGMANAGE_BOOTSTRAP}"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_INSTALL += "start-scripts-firmware-links"
#IMAGE_INSTALL += "start-scripts-find-partitions"
IMAGE_INSTALL += "start-scripts-misc-daemon"
IMAGE_INSTALL += "start-scripts-update-perms"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util', '', d)}"
IMAGE_INSTALL += "start-scripts-recovery-updater"
#IMAGE_INSTALL += "sec-config"

IMAGE_INSTALL += "base-files"
IMAGE_INSTALL += "base-passwd"
IMAGE_INSTALL += "binder"

DEPENDS_append_aarch64 += " gdb-cross-aarch64"

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
IMAGE_INSTALL += "wireless-tools"
IMAGE_INSTALL += "volatile-binds"

#32-bit only packages
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-libnl', 'libnl', d)}"
#IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-gtest', 'gtest', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('MULTILIB_VARIANTS', 'lib32', 'lib32-cairo', 'cairo', d)}"

IMAGE_INSTALL += "libpagemap"
IMAGE_INSTALL += "procrank"

IMAGE_INSTALL += "libuv"
IMAGE_INSTALL += "libwebsockets"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)

IMAGE_EXT4_SELINUX_OPTIONS = "${@bb.utils.contains('DISTRO_FEATURES','selinux',' -S ${SELINUX_FILE_CONTEXTS} ','',d)}"

do_makesystem() {
    cp ${THISDIR}/${BASEMACHINE}/apq8053-fsconfig.conf ${WORKDIR}/rootfs-fsconfig.conf
    make_ext4fs -B ${DEPLOY_DIR_IMAGE}/system.map  -C ${WORKDIR}/rootfs-fsconfig.conf ${IMAGE_EXT4_EXTRA_OPTIONS} -s -a / ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
    make_ext4fs -a /systemrw ${IMAGE_EXT4_SELINUX_OPTIONS} -b 4096 -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMRWIMAGE_TARGET}
    make_ext4fs -l ${RECOVERYFS_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${RECOVERYIMAGE_TARGET}
}
