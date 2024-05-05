inherit autotools pkgconfig

PR = "r0"

DESCRIPTION = "Android verified boot lib"
LICENSE = "Apache-2.0 & BSD-3-Clause & MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10 \
                    file://${COREBASE}/meta/files/common-licenses/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = "file://external/avb/"
S = "${WORKDIR}/external/avb/"

EXTRA_OECONF += " --disable-static"
FILES_${PN} = "${libdir}/libavb.so.*"
