inherit autotools
DESCRIPTION = "OpenMAX core for MSM chipsets"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://mm-video-oss/mm-core"

PR = "r12"

S = "${WORKDIR}/mm-video-oss/mm-core"

LV = "1.0.0"

PACKAGE_ARCH = "${MACHINE_ARCH}"

EXTRA_OECONF_append = " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include "
EXTRA_OECONF_append = " --enable-target-${BASEMACHINE}=yes "

FILES_${PN} = "\
    /usr/lib/* \
    /usr/bin/*"

#Skips check for .so symlinks
INSANE_SKIP_${PN} = "dev-so"

do_install() {
	oe_runmake DESTDIR="${D}/" LIBVER="${LV}" install
}
