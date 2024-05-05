inherit autotools pkgconfig

DESCRIPTION = "Wrapper library for libinput"
HOMEPAGE = "http://us.codeaurora.org/"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r1"

FILESPATH =+ "${WORKSPACE}/frameworks/:"
SRC_URI = "file://input_helper"

S = "${WORKDIR}/input_helper"

DEPENDS = "libinput libevdev"

CFLAGS += "-I${STAGING_INCDIR}/libevdev-1.0"
