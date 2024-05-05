inherit autotools pkgconfig

DESCRIPTION = "Build scrypt library"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PR = "r0"

SRC_URI = "${CLO_LA_GIT}/platform/external/scrypt;protocol=https;nobranch=1;rev=72691396f9fa84fd39cb72d031750e580e648aa3;destsuffix=scrypt"
SRC_URI += "file://0001-Add-GNU-autotools-build-files.patch"

S = "${WORKDIR}/scrypt"

DEPENDS = "liblog openssl"
RDEPENDS_${PN} = "liblog libcrypto"
EXTRA_OECONF += "${@bb.utils.contains('TUNE_FEATURES','neon','--with-arm-neon','',d)}"
