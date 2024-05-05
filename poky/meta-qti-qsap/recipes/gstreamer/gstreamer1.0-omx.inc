SUMMARY = "OpenMAX IL plugins for GStreamer"
SECTION = "multimedia"
LICENSE = "LGPLv2.1"
HOMEPAGE = "http://www.gstreamer.net/"
DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-bad"

inherit autotools pkgconfig gettext

acpaths = "-I ${S}/common/m4 -I ${S}/m4"

PR = "r1"

GSTREAMER_1_0_OMX_TARGET ?= "bellagio"

EXTRA_OECONF += "--disable-valgrind --with-omx-target=${GSTREAMER_1_0_OMX_TARGET}"

python __anonymous () {
    omx_target = d.getVar("GSTREAMER_1_0_OMX_TARGET", True)
    if omx_target in ['generic', 'bellagio']:
        d.appendVar("CFLAGS", " -I${S}/omx/openmax")
    elif omx_target == "rpi":
        d.setVar("PACKAGE_ARCH", d.getVar("MACHINE_ARCH", True))
}

FILES_${PN} += "${libdir}/gstreamer-1.0/*.so"
FILES_${PN}-dbg += "${libdir}/gstreamer-1.0/.debug"
FILES_${PN}-dev += "${libdir}/gstreamer-1.0/*.la"
FILES_${PN}-staticdev += "${libdir}/gstreamer-1.0/*.a"
FILESEXTRAPATHS_prepend := "${THISDIR}/qti-patches:"

