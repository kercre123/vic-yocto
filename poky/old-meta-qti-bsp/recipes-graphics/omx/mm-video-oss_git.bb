inherit autotools

DESCRIPTION = "OpenMAX video for MSM chipsets"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"
FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://mm-video-oss"

PR = "r19"

DEPENDS = "virtual/kernel"
DEPENDS += "glib-2.0"
DEPENDS += "mm-core-oss"
DEPENDS += "adreno200"
RDEPENDS_${PN} = "mm-video-prop"
INSANE_SKIP = "1"

# Need the kernel headers
PACKAGE_ARCH = "${MACHINE_ARCH}"

S = "${WORKDIR}/mm-video-oss"

LV = "1.0.0"

EXTRA_OECONF_append = " --with-libhardware-headers=${WORKSPACE}/hardware/libhardware "
EXTRA_OECONF_append = " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include "
EXTRA_OECONF_append = " --with-common-includes=${STAGING_INCDIR}"
EXTRA_OECONF_append = " --enable-target-${BASEMACHINE}=yes"

CPPFLAGS += "-I${STAGING_INCDIR}/glib-2.0"
CPPFLAGS += "-I${STAGING_LIBDIR}/glib-2.0/include"

CPPFLAGS += "-I${STAGING_INCDIR}/c++"
CPPFLAGS += "-I${STAGING_INCDIR}/c++/${TARGET_SYS}"

LDFLAGS += "-lglib-2.0"

FILES_${PN} = "\
    /usr/lib/* \
    /usr/bin/* \
    /usr/include/* \
    /usr/share/*"

#Skips check for .so symlinks
INSANE_SKIP_${PN} = "dev-so"

do_install() {
	oe_runmake DESTDIR="${D}/" LIBVER="${LV}" install
	mkdir -p ${STAGING_INCDIR}/mm-core
	install -m 0644 ${S}/mm-core/inc/*.h ${STAGING_INCDIR}/mm-core
}
