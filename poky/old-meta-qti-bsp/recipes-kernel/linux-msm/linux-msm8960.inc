do_defconfig_patch () {
cat >> ${S}/arch/arm/configs/${KERNEL_DEFCONFIG} <<MSM_X11_EXTRACONFIGS
CONFIG_DEVTMPFS=y
CONFIG_DEVTMPFS_MOUNT=y
CONFIG_UNIX98_PTYS=y
CONFIG_DRM=y
CONFIG_MSM_KGSL_DRM=y
# CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE is not set
MSM_X11_EXTRACONFIGS
}

do_patch_append () {
bb.build.exec_func('do_defconfig_patch',d)
}

