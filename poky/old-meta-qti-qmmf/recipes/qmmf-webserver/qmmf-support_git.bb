inherit autotools pkgconfig

DESCRIPTION = "QMMF Webserver support libraries"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

PR = "r0"

DEPENDS = "liblog"
DEPENDS += "libcutils"
DEPENDS += "native-frameworks"
DEPENDS += "system-core"
DEPENDS += "glib-2.0"
DEPENDS += "av-frameworks"
DEPENDS += "live555-qti"
DEPENDS += "qmmf-sdk"
DEPENDS += "mm-mux"
DEPENDS += "vam-lib"
DEPENDS += "vam-test"
DEPENDS += "mm-video-noship"
DEPENDS += "sqlite3"

TARGET_CFLAGS += "-I${STAGING_INCDIR} -I${STAGING_INCDIR}/recorder/"
TARGET_CFLAGS += "-I${STAGING_INCDIR}/mm-osal/include -I${STAGING_INCDIR}/mm-mux"
TARGET_CFLAGS += "-I ${STAGING_INCDIR}/libnl3"
TARGET_CFLAGS += "-I ${STAGING_INCDIR}/player/ -I${STAGING_DIR_HOST}/usr/"
EXTRA_OECONF += " --with-camerahal=${WORKSPACE}/camera/lib/QCamera2/HAL3"
EXTRA_OECONF += " --with-camcommon=${WORKSPACE}/camera/lib/QCamera2/stack/common"
EXTRA_OECONF += " --with-camifaceinc=${WORKSPACE}/camera/lib/QCamera2/stack/mm-camera-interface/inc"
EXTRA_OECONF += " --with-exif=${WORKSPACE}/camera/lib/mm-image-codec/qexif"
EXTRA_OECONF += " --with-omxcore=${WORKSPACE}/camera/lib/mm-image-codec/qomx_core"
EXTRA_OECONF += " --with-openmax=${WORKSPACE}/frameworks/native/include/media/openmax"
EXTRA_OECONF_append = " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"

PACKAGECONFIG[sqlite] = "--with-sqlite,--without-sqlite,sqlite3,"

FILESPATH =+ "${WORKSPACE}/vendor/qcom/opensource/:"
SRC_URI  := "file://qmmf-webserver"

S = "${WORKDIR}/qmmf-webserver"

do_package_qa () {
}

FILES_${PN} = "${libdir}/lib*.so.* ${bindir}/* ${libdir}/pkgconfig/*"
FILES_${PN}-dev = "${libdir}/lib*.so* ${includedir} ${libdir}/*.la ${libdir}/*.a"
FILES_${PN}-dbg = "${libdir}/.debug ${bindir}/.debug"
PACKAGES = "${PN} ${PN}-dev ${PN}-dbg"
