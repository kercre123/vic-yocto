
python do_getpatches() {
    import os

    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf gstreamer1.0-plugins-bad && mkdir gstreamer1.0-plugins-bad \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0001-Makefile.am-don-t-hardcode-libtool-name-when-running.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0001-Makefile.am-don-t-hardcode-libtool-name-when-running.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0001-Prepend-PKG_CONFIG_SYSROOT_DIR-to-pkg-config-output.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0001-Prepend-PKG_CONFIG_SYSROOT_DIR-to-pkg-config-output.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0001-gstreamer-gl.pc.in-don-t-append-GL_CFLAGS-to-CFLAGS.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0001-gstreamer-gl.pc.in-don-t-append-GL_CFLAGS-to-CFLAGS.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0001-mssdemux-improved-live-playback-support.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0001-mssdemux-improved-live-playback-support.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0001-smoothstreaming-implement-adaptivedemux-s-get_live_s.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0001-smoothstreaming-implement-adaptivedemux-s-get_live_s.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0001-smoothstreaming-use-the-duration-from-the-list-of-fr.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0001-smoothstreaming-use-the-duration-from-the-list-of-fr.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/0009-glimagesink-Downrank-to-marginal.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/0009-glimagesink-Downrank-to-marginal.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/avoid-including-sys-poll.h-directly.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/avoid-including-sys-poll.h-directly.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/configure-allow-to-disable-libssh2.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/configure-allow-to-disable-libssh2.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/ensure-valid-sentinels-for-gst_structure_get-etc.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/ensure-valid-sentinels-for-gst_structure_get-etc.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-bad/fix-maybe-uninitialized-warnings-when-compiling-with-Os.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-bad/fix-maybe-uninitialized-warnings-when-compiling-with-Os.patch || pwd)"

    os.system(cmd)
}

addtask getpatches before do_fetch
