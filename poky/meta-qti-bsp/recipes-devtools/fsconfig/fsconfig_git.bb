inherit native autotools-brokensep pkgconfig

PR = "r5"

DESCRIPTION = "fs_config tool from Android"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
HOMEPAGE = "http://android.git.kernel.org/?p=platform/system/core.git"

DEPENDS += "libselinux libcutils"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://OTA/build/tools/fs_config/"

S = "${WORKDIR}/OTA/build/tools/fs_config/"

BBCLASSEXTEND = "native"

EXTRA_OECONF_append_class-target = " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"
EXTRA_OECONF_append_class-native = " --with-core-headers=${STAGING_INCDIR_NATIVE}"
EXTRA_OECONF_append_class-native = " --enable-selinux"
