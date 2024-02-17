DESCRIPTION = "bspatch tool from Android needed for delta updates"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
BSD;md5=3775480a712fc46a69647678acb234cb"

DEPENDS = "bzip2"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://external/bspatch/"

S = "${WORKDIR}/external/bspatch/"

TARGET_CPPFLAGS += "-Iinclude"
TARGET_CXXFLAGS += "-std=c++11 -O3 -Wall -Werror -fPIC"

EXTRA_OEMAKE += "USE_BSDIFF=n PREFIX=${D}/usr"

do_install() {
   mkdir -p ${D}/usr/bin
   install -c -m 755 ${S}/bspatch ${D}/usr/bin
}

FILES_${PN} += "usr/bin/bspatch"
