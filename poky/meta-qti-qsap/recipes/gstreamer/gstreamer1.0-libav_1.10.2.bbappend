
python do_getpatches() {
    import os

    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf gstreamer1.0-libav && mkdir gstreamer1.0-libav \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-libav/0001-Disable-yasm-for-libav-when-disable-yasm.patch?h=gstreamer_1.10 -O gstreamer1.0-libav/0001-Disable-yasm-for-libav-when-disable-yasm.patch  || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-libav/mips64_cpu_detection.patch?h=gstreamer_1.10 -O gstreamer1.0-libav/mips64_cpu_detection.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-libav/workaround-to-build-gst-libav-for-i586-with-gcc.patch?h=gstreamer_1.10 -O gstreamer1.0-libav/workaround-to-build-gst-libav-for-i586-with-gcc.patch || pwd)"

    os.system(cmd)
}

addtask getpatches before do_fetch
