SUMMARY = "FFmpeg is a complete, cross-platform solution to record, convert and stream audio and video."
HOMEPAGE = "http://ffmpeg.org"
BUGTRACKER = "http://ffmpeg.org/trac/ffmpeg"
LICENSE = "LGPLv2.1"
PRIORITY = "optional"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/LGPL-2.1;md5=1a6d268fd218675ffea8be556788b780"

# Package Revision (update whenever recipe is changed)
PR = "r0"

SRC_URI = "git://source.codeaurora.org/quic/le/ffmpeg/;protocol=http;branch=ffmpeg/release/2.3;destsuffix=ffmpeg-2.3;name=ffmpeg-2.3"
SRCREV = "${AUTOREV}"

EXTRA_OEMAKE = ""

FILES_${PN} += "/lib/lib*.so.*"
FILES_${PN} += "/lib/pkgconfig/*"
FILES_${PN}-dev += "/usr/share/*"
FILES_${PN}-dev += "/lib/lib*.so"

EXTRA_CFLAGS +="-fPIC"

#enable hardfloat
EXTRA_CFLAGS +="${@base_conditional('ARM_FLOAT_ABI', 'hard', '-march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a8', '', d)}"

do_configure () {
    ./configure --enable-cross-compile --cross-prefix=${TARGET_PREFIX} \
    --cpu=armv7-a --target-os=linux --sysroot=${STAGING_DIR_TARGET} --arch=${TARGET_ARCH} --disable-mmx \
    --enable-shared --disable-doc --disable-htmlpages --disable-manpages --disable-podpages \
    --disable-txtpages --disable-avdevice --disable-swresample --disable-swscale \
    --disable-postproc --enable-small --disable-avfilter --disable-debug --disable-ffserver --disable-ffplay \
    --extra-cflags="${EXTRA_CFLAGS}" --enable-gpl --disable-network --disable-zlib --disable-ffmpeg --disable-encoders \
    --disable-decoders --disable-muxers --disable-bsfs --disable-devices --disable-protocol=udp \
    --disable-protocol=tcp --disable-protocol=rtp --disable-protocol=pipe --disable-protocol=http \
    --disable-parser=cavsvideo --disable-parser=dca --disable-parser=dirac --disable-parser=dnxhd --disable-parser=mjpeg \
    --disable-parser=mlp --disable-parser=pnm --disable-parser=vp3 --disable-demuxer=amr --disable-demuxer=apc \
    --disable-demuxer=ape --disable-demuxer=ass --disable-demuxer=bethsoftvid --disable-demuxer=bfi \
    --disable-demuxer=c93 --disable-demuxer=daud --disable-demuxer=dnxhd --disable-demuxer=dsicin --disable-demuxer=dxa \
    --disable-demuxer=ffm --disable-demuxer=gsm --disable-demuxer=gxf --disable-demuxer=idcin --disable-demuxer=iff \
    --disable-demuxer=image2 --disable-demuxer=image2pipe --disable-demuxer=ingenient --disable-demuxer=ipmovie \
    --disable-demuxer=lmlm4 --disable-demuxer=mm --disable-demuxer=mmf --disable-demuxer=msnwc_tcp \
    --disable-demuxer=mtv --disable-demuxer=mxf --disable-demuxer=nsv --disable-demuxer=nut \
    --disable-demuxer=oma --disable-demuxer=pva --disable-demuxer=rawvideo --disable-demuxer=rl2 \
    --disable-demuxer=roq --disable-demuxer=rpl --disable-demuxer=segafilm --disable-demuxer=shorten \
    --disable-demuxer=siff --disable-demuxer=smacker --disable-demuxer=sol --disable-demuxer=str \
    --disable-demuxer=thp --disable-demuxer=tiertexseq --disable-demuxer=tta --disable-demuxer=txd \
    --disable-demuxer=vmd --disable-demuxer=voc --disable-demuxer=wc3 --disable-demuxer=wsaud \
    --disable-demuxer=wsvqa --disable-demuxer=xa --disable-demuxer=yuv4mpegpipe --enable-demuxer=matroska \
    --disable-altivec --enable-fft --libdir=${base_libdir} --shlibdir=${base_libdir} \
    --prefix=${base_libdir} --incdir=${includedir}
}

do_install() {
    oe_runmake 'DESTDIR=${D}' install
    # Info dir listing isn't interesting at this point so remove it if it exists.
    if [ -e "${D}${infodir}/dir" ]; then
    rm -f ${D}${infodir}/dir
    fi
}
