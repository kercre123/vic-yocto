inherit androidmk

SUMMARY = "Camera libraries and SDK"
SECTION = "camera"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI   = "file://camera/lib-legacy"
SRC_URI  += "file://mm-anki-camera.service"

SRCREV = "${AUTOREV}"
S      = "${WORKDIR}/lib-legacy"

DEPENDS += "media"
DEPENDS += "glib-2.0"
DEPENDS += "display-hal-linux"

EXTRA_OEMAKE += "TARGET_COMPILE_WITH_MSM_KERNEL=true"
EXTRA_OEMAKE += "SRC_CAMERA_HAL_DIR='${S}'"
EXTRA_OEMAKE += "QC_PROP_ROOT='${TMPDIR}'/work/'${MULTIMACH_TARGET_SYS}'/'${MLPREFIX}'camerabackend/'${EXTENDPE}${PV}-${PR}'/service"
EXTRA_OEMAKE += "TARGET_IS_HEADLESS=true"
EXTRA_OEMAKE += "TARGET_USES_AOSP=false"
EXTRA_OEMAKE += "OMX_HEADER_DIR='${TMPDIR}'/work/'${MULTIMACH_TARGET_SYS}'/'${MLPREFIX}'media/'${EXTENDPE}${PV}-${PR}'/hardware/qcom/media/mm-core/inc"

CFLAGS += "-Wno-error -Wno-uninitialized -Wno-error=attributes -Wno-error=unused-parameter"
CFLAGS += "-Wno-error=builtin-macro-redefined -Wno-error=type-limits"
CFLAGS += "-D__unused="__attribute__((__unused__))""
CFLAGS += "-D_GNU_SOURCE"
CFLAGS += "-D_LE_CAMERA_"
CFLAGS += "-DUNIX_PATH_MAX=108"

CFLAGS += "-I $(OMX_HEADER_DIR)"
CFLAGS += "-I${STAGING_INCDIR}/glib-2.0"
CFLAGS += "-I${STAGING_LIBDIR}/glib-2.0/include"

CFLAGS += "-include stdint.h"
CFLAGS += "-Dstrlcpy=g_strlcpy"
CFLAGS += "-Dstrlcat=g_strlcat"
#CFLAGS += "-include glib.h"
#CFLAGS += "-include glibconfig.h"
CFLAGS += "-include sys/ioctl.h"

LDFLAGS += "-lcutils"
LDFLAGS += "-lglib-2.0"
LDFLAGS += "-llog"
LDFLAGS += "-lbase"
LDFLAGS += "-lutils"
LDFLAGS += "-lsync"
LDFLAGS += "-lbinder"
LDFLAGS += "-lcamera_client"
LDFLAGS += "-lcamera_metadata"
LDFLAGS += "-lqservice"
LDFLAGS += "-lqdMetaData"
LDFLAGS += "-lhardware"
LDFLAGS += "-lrt"

do_fixup_unpack () {
    # Replace all the $(LOCAL_PATH)/../../../hardware/qcom/camera
    find ${S}/ -type f -name "*.mk" -exec sed -i 's/\$.*\/hardware\/qcom\/camera/\$\(SRC_CAMERA_HAL_DIR\)/g' {} +

    # Replace all the hardware/qcom/camera
    find ${S}/ -type f -name "*.mk" -exec sed -i 's/\/hardware\/qcom\/camera/\$\(SRC_CAMERA_HAL_DIR\)/g' {} +
}
addtask fixup_unpack after do_patch before do_populate_lic

export TARGET_LIBRARY_SUPPRESS_LIST="libcamera_client libhardware \
        libsync libui libcamera_metadata libqdMetaData libqservice \
        libbinder libgui libstlport libandroid"

do_compile () {
    # Current support is limited to 32-bit build
    if [ "${MLPREFIX}" == "lib32-" ] || [ "${MLPREFIX}" == "" -a "${TUNE_ARCH}" == "arm" ]; then
        androidmk_setenv
        export TARGET_SUPPORT_HAL1=false
        oe_runmake -f ${LA_COMPAT_DIR}/build/core/main.mk BUILD_MODULES_IN_PATHS=${S} \
            all_modules SHOW_COMMANDS=true || die "make failed"
    else
        die "64-bit compilation not supported"
    fi
}

do_install_append() {
   mkdir -p ${STAGING_INCDIR}/cameracommon
   cp -rf ${S}/*.h ${STAGING_INCDIR}/cameracommon
}

do_install () {
    androidmk_setenv

    export TARGET_OUT_HEADERS=${D}${includedir}
    export TARGET_OUT_VENDOR_SHARED_LIBRARIES=${D}${libdir}
    export TARGET_OUT_SHARED_LIBRARIES=${D}${libdir}
    export TARGET_OUT_EXECUTABLES=${D}${bindir}
    export TARGET_SUPPORT_HAL1=false

    oe_runmake -f ${LA_COMPAT_DIR}/build/core/main.mk BUILD_MODULES_IN_PATHS=${S} \
        all_modules SHOW_COMMANDS=true USE_INSTALL=true || die "make failed"

   if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
       install -d ${D}${systemd_unitdir}/system/
       install -m 0644 ${WORKDIR}/mm-anki-camera.service -D ${D}${systemd_unitdir}/system/mm-anki-camera.service
       install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
       install -d ${D}${systemd_unitdir}/system/ffbm.target.wants/
       # enable the service for multi-user.target
       ln -sf ${systemd_unitdir}/system/mm-anki-camera.service \
            ${D}${systemd_unitdir}/system/multi-user.target.wants/mm-anki-camera.service
       # enable the service for ffbm.target
       ln -sf ${systemd_unitdir}/system/mm-anki-camera.service \
            ${D}${systemd_unitdir}/system/ffbm.target.wants/mm-anki-camera.service
   fi
}

FILES_${PN} += "${systemd_unitdir}/system/"
