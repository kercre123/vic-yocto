
python do_getpatches() {
    import os

    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf files gst-player gstreamer1.0  \
    && mkdir files gst-player gstreamer1.0 \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0/deterministic-unwind.patch?h=gstreamer_1.10 -O gstreamer1.0/deterministic-unwind.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/files/0001-introspection.m4-prefix-pkgconfig-paths-with-PKG_CON.patch?h=gstreamer_1.10 -O files/0001-introspection.m4-prefix-pkgconfig-paths-with-PKG_CON.patch || pwd)"

    os.system(cmd)
}

addtask getpatches before do_fetch
