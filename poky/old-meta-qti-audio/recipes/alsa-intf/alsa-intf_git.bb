inherit autotools pkgconfig

DESCRIPTION = "ALSA Framework Library"
LICENSE = "Apache-2.0"
PR = "r5"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
DEPENDS = "acdbloader glib-2.0"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://qcom-opensource/mm-audio/"

S = "${WORKDIR}/qcom-opensource/mm-audio/"

EXTRA_OECONF += "--prefix=/etc \
                 --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
                 --with-glib \
                 --with-acdb"

FILES_${PN} += "${prefix}/snd_soc_msm/*"

do_install_append_msm8610() {
    mv ${D}/usr/bin/aplay ${D}/usr/bin/qc-aplay
    mv ${D}/usr/bin/amix ${D}/usr/bin/qc-amix
    mv ${D}/usr/bin/arec ${D}/usr/bin/qc-arec
    mv ${D}/usr/bin/alsaucm_test ${D}/usr/bin/qc-alsaucm_test
}

do_install_append_msm8226() {
    mv ${D}/usr/bin/aplay ${D}/usr/bin/qc-aplay
    mv ${D}/usr/bin/amix ${D}/usr/bin/qc-amix
    mv ${D}/usr/bin/arec ${D}/usr/bin/qc-arec
    mv ${D}/usr/bin/alsaucm_test ${D}/usr/bin/qc-alsaucm_test
}
