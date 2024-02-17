# Enable Bootloader
RDEPENDS += "${@bb.utils.contains('TARGET_ARCH', 'arm','lib64-edk2','edk2',d)}"

# Packages to be included in rootfs creation.
IMAGE_INSTALL += "packagegroup-core-boot"
IMAGE_INSTALL += "${CORE_IMAGE_EXTRA_INSTALL}"

#IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','selinux', 'packagegroup-core-selinux', '', d)}"
#IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','selinux', 'selinux-config', '', d)}"
#IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','selinux', 'refpolicy-mls', '', d)}"

IMAGE_INSTALL += "start-scripts-firmware-links"

#IMAGE_INSTALL += "start-scripts-misc-daemon"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'start-script-ab-slot-util', '', d)}"

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
#IMAGE_INSTALL += "system-conf"
IMAGE_INSTALL += "iw"
IMAGE_INSTALL += "system-prop"
IMAGE_INSTALL += "systemd-machine-units"
#IMAGE_INSTALL += "wireless-tools"
IMAGE_INSTALL += "volatile-binds"
IMAGE_INSTALL += "libhardware"
IMAGE_INSTALL += "binder"
IMAGE_INSTALL += "kernel-modules"
IMAGE_INSTALL += "av-frameworks"
IMAGE_INSTALL += "libstdc++"
IMAGE_INSTALL += "coreutils"
IMAGE_INSTALL += "media-headers"
IMAGE_INSTALL += "protobuf"
IMAGE_INSTALL += "openssh"

#Install virtualization support packages
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'docker docker-registry python3-docker-compose', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'cgroup-lite aufs-util xz ca-certificates chrony', '', d)}"

# Install recovery and its libs in system image if A/B boot is supported
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'recovery-ab-bin', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', 'recovery-ab', '', d)}"

# Below is to generate sparse ext4 system image (OE by default supports raw ext4 images)
do_makesystem() {
    cp ${THISDIR}/${BASEMACHINE}/sdm845-fsconfig.conf ${WORKDIR}/rootfs-fsconfig.conf
    make_ext4fs -B ${DEPLOY_DIR_IMAGE}/system.map -C ${WORKDIR}/rootfs-fsconfig.conf ${IMAGE_EXT4_EXTRA_OPTIONS} -s -a / ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEM_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET} ${IMAGE_ROOTFS}
    make_ext4fs -a /systemrw ${IMAGE_EXT4_SELINUX_OPTIONS} -l ${SYSTEMRW_SIZE_EXT4} ${DEPLOY_DIR_IMAGE}/${SYSTEMRWIMAGE_TARGET}
}
