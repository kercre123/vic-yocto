inherit androidcompat androidmk_base

SUMMARY = "Android compat build framework"
SECTION = "adaptors"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9"

DEPENDS = "liblog libcutils system-core"

FILESPATH =+ "${WORKSPACE}:"

# use a local cloned repo
SRC_URI   = "file://android_compat"

SRCREV = "${AUTOREV}"
S = "${WORKDIR}/android_compat"

PROVIDES += "virtual/androidcompat"

EXTRA_OECONF = " --with-core-includes=${WORKSPACE}/system/core/include --with-glib"
CFLAGS += "-I${STAGING_INCDIR}/cutils"
LDFLAGS += "-lcutils"

#enable hardfloat
EXTRA_OEMAKE_append = " ${@base_conditional('ARM_FLOAT_ABI', 'hard', 'ENABLE_HARD_FPU=1', '', d)}"
