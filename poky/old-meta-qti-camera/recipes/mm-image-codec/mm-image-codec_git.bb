inherit autotools

DESCRIPTION = "mm-image-codec"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PV = "1.0.0"
PR = "r2"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://camera-hal/mm-image-codec"

S = "${WORKDIR}/camera-hal/mm-image-codec"

# Need the kernel headers
DEPENDS += "virtual/kernel"
DEPENDS += "dlog"

PACKAGE_ARCH = "${MACHINE_ARCH}"

EXTRA_OECONF_append = " --enable-debug=no --with-dlog"
EXTRA_OECONF_append = " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"
EXTRA_OECONF_append = " --with-omx-includes=${WORKSPACE}/mm-video-oss/mm-core/inc"
EXTRA_OECONF_append = " --enable-target=${BASEMACHINE}"

FILES_${PN} += "\
    /usr/lib/* "

# The mm-still package contains symlinks that trip up insane
INSANE_SKIP_${PN} = "dev-so"
