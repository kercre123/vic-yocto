inherit qimage

#  Defined in qimage.bbclass. Following is the order of priority.
#  P1: <basemachine>/<basemachine>-<distro>-base-image.inc
#  P2: <basemachine>/<basemachine>-base-image.inc
#  P3: common/common-base-image.inc
include ${@get_bblayer_img_inc('base', d)}

require include/mdm-bootimg.inc
DEPENDS += " mkbootimg-native"
DEPENDS += "squashfs-tools-native"

require include/mdm-ota-target-image-ubi.inc
require include/mdm-ota-target-image-ext4.inc

MULTILIBRE_ALLOW_REP =. "/usr/include/python2.7/*|${base_bindir}|${base_sbindir}|${bindir}|${sbindir}|${libexecdir}|${sysconfdir}|${nonarch_base_libdir}/udev|/lib/modules/[^/]*/modules.*|"

do_fsconfig() {
 chmod go-r ${IMAGE_ROOTFS}/etc/passwd || :
 chmod -R o-rwx ${IMAGE_ROOTFS}/etc/init.d/ || :
 if [ "${DISTRO_NAME}" == "msm-user" ]; then
  if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
    rm ${IMAGE_ROOTFS}/lib/systemd/system/sys-kernel-debug.mount
  else
    sed -i '/mount -t debugfs/ d' ${IMAGE_ROOTFS}/etc/init.d/sysfs.sh
  fi
 fi
}

ROOTFS_POSTPROCESS_COMMAND += "do_fsconfig; "
