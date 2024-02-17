inherit native autotools pkgconfig

DESCRIPTION = "Android system/core components for linux"
HOMEPAGE = "http://developer.android.com/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

PR = "r0"
DEPENDS += "zlib libcutils-native liblog-native"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI   = "file://system/core/"

S = "${WORKDIR}/system/core"

EXTRA_OECONF = " --with-host-os=${HOST_OS}"

#TODO: Install host bins
do_install_append () {
	install -d ${DEPLOY_DIR}/host/linux/bin
	#install ${D}/usr/bin/adb ${DEPLOY_DIR}/host/linux/bin
	#install ${D}/usr/bin/fastboot ${DEPLOY_DIR}/host/linux/bin
}

