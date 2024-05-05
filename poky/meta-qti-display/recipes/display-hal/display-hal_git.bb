inherit autotools qcommon

DESCRIPTION = "display Library"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r8"

PACKAGES = "${PN}"

SRC_DIR = "${WORKSPACE}/display/display-hal/"
S = "${WORKDIR}/display/display-hal/"

DEPENDS += "system-core"
DEPENDS += "libhardware"
DEPENDS += "native-frameworks"

EXTRA_OECONF = " --with-core-includes=${WORKSPACE}/system/core/include"
EXTRA_OECONF += " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"

LDFLAGS += "-llog -lhardware -lutils -lcutils"

CPPFLAGS += "-DTARGET_HEADLESS"
CPPFLAGS += "-DVENUS_COLOR_FORMAT"
CPPFLAGS += "-DPAGE_SIZE=4096"
CPPFLAGS += "-I${SRC_DIR}/libqdutils"
CPPFLAGS += "-I${SRC_DIR}/libqservice"
CPPFLAGS += "-I${SRC_DIR}/sdm/include"
CPPFLAGS += "-I${SRC_DIR}/include"
CPPFLAGS += "-I${SRC_DIR}/libgralloc"
CPPFLAGS += "-I${WORKSPACE}/system/core/include"

# Need to revisit
# libcamera and libadreno giving compilation errors
# so exporting libqservice headers and qdMetaData.h to ${D}${includedir}
do_install_append () {
    install -d ${D}${includedir}
    install -m 0644 ${S}/libgralloc/gralloc_priv.h -D ${D}${includedir}/gralloc_priv.h
    install -m 0644 ${S}/libqdutils/qdMetaData.h   -D ${D}${includedir}/libqdutils/qdMetaData.h
    install -m 0644 ${S}/libqdutils/qdMetaData.h   -D ${D}${includedir}
    install -m 0644 ${S}/libqservice/*.h   -D ${D}${includedir}
    # libhardware expects to find /usr/lib/hw/gralloc.*.so
    install -d ${D}/usr/lib/hw
    ln -s /usr/lib/libgralloc.so ${D}/usr/lib/hw/gralloc.default.so
}

# Both libhardware and display-hal provides gralloc_priv.h
# However display-hal's header has vendor specific definitions
# and it should be used whenever available.
do_fix_sysroot () {
   if [ -f ${STAGING_INCDIR}/gralloc_priv.h ]; then
      rm ${STAGING_INCDIR}/gralloc_priv.h
   else
      echo "${STAGING_INCDIR}/gralloc_priv.h not found"
   fi
}
addtask fix_sysroot after do_install before do_populate_sysroot

FILES_${PN} = "${libdir}/*.so"
FILES_${PN} += "${libdir}/hw/gralloc.default.so"
INSANE_SKIP_${PN} = "dev-so"
