require recipes-graphics/xorg-driver/xorg-driver-common.inc

DESCRIPTION = "X.Org X server -- MSM display driver"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=3775480a712fc46a69647678acb234cb"
#PV = "git-${GITSHA}"

PR = "r9"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://xf86-video-msm"
S = "${WORKDIR}/xf86-video-msm"

EXTRA_OECONF_append += " --with-kernel-headers=${STAGING_KERNEL_DIR}/usr/include \
                         --with-c2d=${STAGING_EXECPREFIXDIR} \
                         --with-c2d-headers=${STAGING_INCDIR}/C2D \
                         --with-c2d-libraries=${STAGING_LIBDIR}"

EXTRA_OECONF_append += " --enable-target=${BASEMACHINE}"

RDEPENDS_${PN} += "xserver-xorg"

DEPENDS = "${RDEPENDS} \
           dri2proto \
           fontsproto \
           glproto \
           libdri2 \
           libtbm \
           randrproto \
           renderproto \
           videoproto \
           virtual/kernel \
           xdbg \
           xf86driproto \
           xproto \
           adreno200"

#TODO: remove this once adreno200 symlinks are fixed
INSANE_SKIP_${PN} = "dev-deps"

PACKAGE_ARCH = "${MACHINE_ARCH}"

ARM_INSTRUCTION_SET = "arm"
