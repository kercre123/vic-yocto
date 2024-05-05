
python do_getpatches() {
    import os

    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf gstreamer1.0-rtsp-server  && mkdir gstreamer1.0-rtsp-server \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-rtsp-server/0001-Don-t-hardcode-libtool-name-when-using-introspection.patch?h=gstreamer_1.10 -O gstreamer1.0-rtsp-server/0001-Don-t-hardcode-libtool-name-when-using-introspection.patch || pwd)"

    os.system(cmd)
}

addtask getpatches before do_fetch