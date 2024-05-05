include gstreamer1.0-omx.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=4fbd65380cdd255951079008b364516c \
                    file://omx/gstomx.h;beginline=1;endline=21;md5=5c8e1fca32704488e76d2ba9ddfa935f"

SRC_URI = "http://gstreamer.freedesktop.org/src/gst-omx/gst-omx-${PV}.tar.xz"
SRC_URI += "file://0001-omxaudioenc-set-base-class-format-instead-of-just-so.patch"
SRC_URI += "file://0001-omxaacenc-let-encoder-know-about-incoming-rate-chann.patch"
SRC_URI += "file://0001-omxaacenc-fix-samples-per-buffer-calculation.patch"
SRC_URI += "file://0001-Enabling-omx-aac-encoder-component.patch"

SRC_URI[md5sum] = "d8395c2d7bbe05517cd20f608813f03c"
SRC_URI[sha256sum] = "c069a9cf775c92f889ca8f3b2fc718e428cd0579b7b805851a960c850a7aa497"

S = "${WORKDIR}/gst-omx-${PV}"

do_install_append () {
    install -d ${D}${includedir}/gstreamer-1.0/gst/gst-omx/
    install -D ${S}/omx/*.h ${D}${includedir}/gstreamer-1.0/gst/gst-omx
    install -D ${S}/omx/openmax/*.* ${D}${includedir}/gstreamer-1.0/gst/gst-omx
}
