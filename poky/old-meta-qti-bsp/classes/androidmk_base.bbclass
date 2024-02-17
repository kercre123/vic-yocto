DEPENDS += "virtual/kernel"

LA_COMPAT_DIR ?= "${TMPDIR}/work-shared/${BASEMACHINE}/android_compat"
LA_OUT_DIR ?= "${TMPDIR}/work-shared/${BASEMACHINE}/${MLPREFIX}android_compat_build_artifacts"
LA_TARGET_PRODUCT ?= "${SOC_FAMILY}"
LA_OUT_TARGET_INTERMEDIATES = "${LA_OUT_DIR}/target/product/${LA_TARGET_PRODUCT}/obj"

# androidmk installs only stripped binaries and shared libraries. the below 
# skip is to turn off related QA error.
INSANE_SKIP_${PN} = "already-stripped"
INSANE_SKIP_${PN} += "dev-deps"

# turn off QA error: "No GNU_HASH in the elf binary"
INSANE_SKIP_${PN}-dev = "ldflags"

androidmk_setenv() {
    export TOP=${S}
    export OUT_DIR=${LA_OUT_DIR}
    export ANDROID_COMPAT_DIR=${LA_COMPAT_DIR}
    export TARGET_SYSROOT=${PKG_CONFIG_SYSROOT_DIR}
    export TARGET_TOOLS_PREFIX=${STAGING_BINDIR_NATIVE}/${TARGET_SYS}/${TARGET_PREFIX}
    export WITHOUT_CHECK_API=true
    export TARGET_PRODUCT=${LA_TARGET_PRODUCT}
    export TARGET_BUILD_VARIANT=userdebug

    # Assume lib64 if multilib variant is unknown
    #
    if [ "${MLPREFIX}" == "lib64-" ]; then
        bbnote "building for lib64-"

        export TARGET_CPU_ABI=arm64-v8a
        export TARGET_ARCH_VARIANT=armv8-a
        export TARGET_ARCH=arm64
        export TARGET_CPU_VARIANT=generic
    elif [ "${MLPREFIX}" == "lib32-" ] || [ "${MLPREFIX}" == "lib32a7-" ]; then
        bbnote "building for lib32-"

        export TARGET_CPU_ABI=armeabi-v7a
        export TARGET_ARCH_VARIANT=armv7-a-neon
        export TARGET_ARCH=arm
        export TARGET_CPU_VARIANT=cortex-a15
    elif [ "${MLPREFIX}" == "" ]; then
        if [ "${TUNE_ARCH}" == "arm" ]; then
            bbnote "building for default TUNE_ARCH (arm)"

            export TARGET_CPU_ABI=armeabi-v7a
            export TARGET_ARCH_VARIANT=armv7-a-neon
            export TARGET_ARCH=arm
            export TARGET_CPU_VARIANT=cortex-a15
        elif [ "${TUNE_ARCH}" == "aarch64" ]; then
            bbnote "building for default TUNE_ARCH (aarch64)"

            export TARGET_CPU_ABI=arm64-v8a
            export TARGET_ARCH_VARIANT=armv8-a
            export TARGET_ARCH=arm64
            export TARGET_CPU_VARIANT=generic
        fi

    else
        die "not supported"
    fi

    # Establish a soft link to kernel headers
    #
    if [ ! -d ${LA_OUT_TARGET_INTERMEDIATES} ]; then
        mkdir -p ${LA_OUT_TARGET_INTERMEDIATES}
        ln -s ${STAGING_KERNEL_BUILDDIR} ${LA_OUT_TARGET_INTERMEDIATES}/KERNEL_OBJ
    fi
}

do_compile() {
    androidmk_setenv
    oe_runmake -f ${LA_COMPAT_DIR}/build/core/main.mk BUILD_MODULES_IN_PATHS=${S} \
        all_modules SHOW_COMMANDS=true || die "make failed"
}

FILES_${PN} = " \
     /system/* \
     ${libdir}/* \
     ${bindir}/* \
     "
FILES_${PN}-dev = " \
     ${includedir}/* \
     "
FILES_${PN}-dbg = " \
    ${libdir}/.debug/* \
    ${bindir}/.debug/* \
    ${sbindir}/.debug/* \
    ${libdir}/${PN}/.debug/* \
    "

do_install () {
    # usual env for androidmk
    androidmk_setenv

    # install in to ${D}
    export TARGET_OUT_HEADERS=${D}${includedir}
    export TARGET_OUT_VENDOR_SHARED_LIBRARIES=${D}${libdir}
    export TARGET_OUT_SHARED_LIBRARIES=${D}${libdir}
    export TARGET_OUT_EXECUTABLES=${D}${bindir}

    # android build uses cp instead of install, set USE_INSTALL=true 
    oe_runmake -f ${LA_COMPAT_DIR}/build/core/main.mk BUILD_MODULES_IN_PATHS=${S} \
        all_modules SHOW_COMMANDS=true USE_INSTALL=true || die "make failed"
}

do_configure[noexec] = "1"
