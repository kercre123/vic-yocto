
python do_getpatches() {
    import os
    #We are using wget as patches are not available in .patch format in a https URI
    #wget command is 'OR'ed with pwd as sometimes wget returns an error even though the patch is fetched succesfully. Hence pwd helps in ignoring 'false failure'
    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf gstreamer1.0-omx && mkdir gstreamer1.0-omx \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-omx/patch/?id=e8e158fde42cae9370bccf3d307091057b15c1d2 -O gstreamer1.0-omx/0001-omxaudioenc-set-base-class-format-instead-of-just-so.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-omx/patch/?id=601eb4f98033c9f25f48871d823ae407af448005 -O gstreamer1.0-omx/0001-omxaacenc-let-encoder-know-about-incoming-rate-chann.patch || pwd) \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-omx/patch/?id=2d1138f45cb482fcf0e2aba5ec40bd4c1cdb5853 -O gstreamer1.0-omx/0001-omx-fixed-type-error-in-printf-call.patch || pwd) "

    os.system(cmd)
}

addtask getpatches before do_fetch
