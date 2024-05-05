
python do_getpatches(){
    import os

    cmd = "cd ${WORKSPACE}/poky/meta-qti-qsap/recipes/gstreamer \
    && rm -rf  gstreamer1.0-plugins-good && mkdir gstreamer1.0-plugins-good \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-good/0001-gstrtpmp4gpay-set-dafault-value-for-MPEG4-without-co.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-good/0001-gstrtpmp4gpay-set-dafault-value-for-MPEG4-without-co.patch   || pwd) \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-good/avoid-including-sys-poll.h-directly.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-good/avoid-including-sys-poll.h-directly.patch  || pwd)  \
    && (wget https://source.codeaurora.org/quic/qyocto/oss/poky/plain/meta/recipes-multimedia/gstreamer/gstreamer1.0-plugins-good/ensure-valid-sentinel-for-gst_structure_get.patch?h=gstreamer_1.10 -O gstreamer1.0-plugins-good/ensure-valid-sentinel-for-gst_structure_get.patch   || pwd) \
    && (wget https://source.codeaurora.org/quic/le/gstreamer/gst-plugins-good/patch/?id=1eecddd040ec59f8adc25af478a28ba01ffdf348 -O gstreamer1.0-plugins-good/qtdemux-free-seqh-after-calling-qtdemux_parse_svq3_s.patch  || pwd)"

    os.system(cmd)
}

addtask getpatches before do_fetch