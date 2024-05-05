
python do_getpatches() {
    import os

    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf gstreamer1.0-plugins-base && mkdir gstreamer1.0-plugins-base  \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-plugins-base/patch/?id=6134dab3bba13674f82c5befae7e459a67d979ad -O gstreamer1.0-plugins-base/Add-Flac.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-plugins-base/patch/?id=c46607095ea5dab7bf42d7b6d84f090eaaa681f1 -O gstreamer1.0-plugins-base/0001-audioringbuffer-Also-support-raw-AAC.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-plugins-base/patch/?id=6ee5922f2f93e5e24a68f2c58859b3474920c23b -O gstreamer1.0-plugins-base/0001-audioringbuffer-do-not-require-4-byte-multiple-for-e.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/0001-Makefile.am-don-t-hardcode-libtool-name-when-running.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/0001-Makefile.am-don-t-hardcode-libtool-name-when-running.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/0002-Makefile.am-prefix-calls-to-pkg-config-with-PKG_CONF.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/0002-Makefile.am-prefix-calls-to-pkg-config-with-PKG_CONF.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/0003-riff-add-missing-include-directories-when-calling-in.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/0003-riff-add-missing-include-directories-when-calling-in.patch  || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/0003-ssaparse-enhance-SSA-text-lines-parsing.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/0003-ssaparse-enhance-SSA-text-lines-parsing.patch  || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/0004-rtsp-drop-incorrect-reference-to-gstreamer-sdp-in-Ma.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/0004-rtsp-drop-incorrect-reference-to-gstreamer-sdp-in-Ma.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/0004-subparse-set-need_segment-after-sink-pad-received-GS.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/0004-subparse-set-need_segment-after-sink-pad-received-GS.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/encodebin-Need-more-buffers-in-output-queue-for-bett.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/encodebin-Need-more-buffers-in-output-queue-for-bett.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/get-caps-from-src-pad-when-query-caps.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/get-caps-from-src-pad-when-query-caps.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-base/make-gio_unix_2_0-dependency-configurable.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-base/make-gio_unix_2_0-dependency-configurable.patch  || pwd)"

    os.system(cmd)
}

addtask getpatches before do_fetch