# List of packages installed onto the root file system as specified by the user.
include ${BASEMACHINE}/${BASEMACHINE}-minimal-image.inc

require include/mdm-bootimg.inc

require include/mdm-ota-target-image-ubi.inc
require include/mdm-ota-target-image-ext4.inc

inherit core-image

MULTILIBRE_ALLOW_REP =. "/usr/include/python2.7/*|"

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
