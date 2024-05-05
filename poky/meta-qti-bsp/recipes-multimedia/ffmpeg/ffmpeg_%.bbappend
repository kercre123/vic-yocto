EXTRA_OEMAKE = ""

FILES_${PN} += "/lib/lib*.so.*"
FILES_${PN} += "/lib/pkgconfig/*"
FILES_${PN}-dev += "/usr/share/*"
FILES_${PN}-dev += "/lib/lib*.so"

PACKAGECONFIG = "avdevice avfilter avcodec avformat swresample swscale postproc bzlib gpl theora"

# Support multilib compilation for libav
PROVIDES += "${MLPREFIX}libav"

EXTRA_CFLAGS_append += " -fPIC"
EXTRA_CFLAGS_append += " ${@ bb.utils.contains('TUNE_FEATURES', 'callconvention-hard', '-mfloat-abi=hard', '', d)}"
EXTRA_CFLAGS_append += " ${@ bb.utils.contains('TUNE_FEATURES', 'neon', '-mfpu=neon', '', d)}"
EXTRA_CFLAGS_append += " ${@ bb.utils.contains('TUNE_FEATURES', 'armv7a', '-march=armv7-a', '', d)}"
EXTRA_CFLAGS_append += " ${@ bb.utils.contains('TUNE_FEATURES', 'cortexa8', '-mtune=cortex-a8', '', d)}"

EXTRA_OECONF_append += " \
    --target-os=linux --sysroot=${STAGING_DIR_TARGET} --arch=${TARGET_ARCH} --disable-mmx \
    --enable-shared --disable-doc --disable-htmlpages --disable-manpages --disable-podpages \
    --disable-txtpages --enable-small --disable-debug --disable-ffplay  \
    --extra-cflags="${EXTRA_CFLAGS}" --disable-network --disable-zlib --disable-ffmpeg \
    --disable-muxers --disable-bsfs --disable-devices --disable-protocol=udp \
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
    --prefix=${base_libdir} --incdir=${includedir} \
"

do_install() {
    oe_runmake 'DESTDIR=${D}' install
    # Info dir listing isn't interesting at this point so remove it if it exists.
    if [ -e "${D}${infodir}/dir" ]; then
    rm -f ${D}${infodir}/dir
    fi
}
