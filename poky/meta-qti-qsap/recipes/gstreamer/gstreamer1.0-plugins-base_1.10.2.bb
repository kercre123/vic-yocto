require gstreamer1.0-plugins-base.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=c54ce9345727175ff66d17b67ff51f58 \
                    file://COPYING.LIB;md5=6762ed442b3822387a51c92d928ead0d \
                    file://common/coverage/coverage-report.pl;beginline=2;endline=17;md5=a4e1830fce078028c8f0974161272607"

SRC_URI = " \
    http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-${PV}.tar.xz \
    file://get-caps-from-src-pad-when-query-caps.patch \
    file://0003-ssaparse-enhance-SSA-text-lines-parsing.patch \
    file://0004-subparse-set-need_segment-after-sink-pad-received-GS.patch \
    file://encodebin-Need-more-buffers-in-output-queue-for-bett.patch \
    file://make-gio_unix_2_0-dependency-configurable.patch \
    file://0001-introspection.m4-prefix-pkgconfig-paths-with-PKG_CON.patch \
    file://0001-audioringbuffer-do-not-require-4-byte-multiple-for-e.patch \
    file://0001-audioringbuffer-Also-support-raw-AAC.patch \
    file://Add-Flac.patch \
    file://Add-Vorbis.patch \
    file://0001-audioringbuffer-add-wma-and-alac-to-encoded-audio-fo.patch \
    file://0002-audioringbuffer-Fix-8kHz-MP3-playback-issue.patch \
"
SRC_URI[md5sum] = "8efa9e9ad9a841a900359604da82fb8b"
SRC_URI[sha256sum] = "fbc0d40fcb746d2efe2ea47444674029912f66e6107f232766d33b722b97de20"

S = "${WORKDIR}/gst-plugins-base-${PV}"
