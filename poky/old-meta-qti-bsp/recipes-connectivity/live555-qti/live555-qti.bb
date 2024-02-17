SUMMARY = "Live555 streaming protocol"
SECTION = "camera"
LICENSE = "LGPLv2.1"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/COPYING;md5=68ad62c64cc6c620126241fd429e68fe"

# fetch from server
SRC_URI = "git://source.codeaurora.org/quic/le/live555;protocol=http;branch=github/master"
SRC_URI += "file://config.linux-cross"
SRC_URI += "file://0001-live555-Update-fps-and-bank-size-value.patch"
SRC_URI += "file://0002-live555-Decrease-RTP-buffer-size.patch"
SRC_URI += "file://0003-live555-Avoid-instabilities-in-MPEG2TransportStreamF.patch"
SRC_URI += "file://0004-live555-Add-support-for-metadata-streaming.patch"
SRC_URI += "file://0001-live555-Add-support-for-server-port-reuse.patch"
SRC_URI += "file://0006-live555-Remove-computing-fDurationInMicroseconds.patch"
SRC_URI += "file://0007-live555-Rewrite-H264or5VideoStreamDiscreteFramer.patch"
# commit d9e97d7953d531a243a8372870bdb6c7a9bb80cb
SRCREV = "d9e97d7953d531a243a8372870bdb6c7a9bb80cb"

S = "${WORKDIR}/git"

do_configure() {
    cp ${WORKDIR}/config.linux-cross .
    ./genMakefiles linux-cross
}

do_compile() {
    make
}

do_install() {
    install -d ${D}${includedir}/BasicUsageEnvironment
    install -d ${D}${includedir}/groupsock
    install -d ${D}${includedir}/liveMedia
    install -d ${D}${includedir}/UsageEnvironment
    install -d ${D}${libdir}
    cp -a ${S}/BasicUsageEnvironment/include/*.hh ${D}${includedir}/BasicUsageEnvironment/
    cp -a ${S}/groupsock/include/*.h ${D}${includedir}/groupsock/
    cp -a ${S}/groupsock/include/*.hh ${D}${includedir}/groupsock/
    cp -a ${S}/liveMedia/include/*.hh ${D}${includedir}/liveMedia/
    cp -a ${S}/UsageEnvironment/include/*.hh ${D}${includedir}/UsageEnvironment/
    # Find all the headers
    for i in $(find . -name "*.hh") $(find . -name "*.h") ; do
        install ${i} ${D}${includedir}
    done
    cp ${S}/*/*.so ${D}${libdir}
}

FILES_${PN}-libBasicUsageEnvironment-dbg    = "${libdir}/.debug/libBasicUsageEnvironment.*"
FILES_${PN}-libBasicUsageEnvironment        = "${libdir}/libBasicUsageEnvironment.so"
FILES_${PN}-libBasicUsageEnvironment-dev    = "${libdir}/libBasicUsageEnvironment.so ${includedir}"

FILES_${PN}-libgroupsock-dbg    = "${libdir}/.debug/libgroupsock.*"
FILES_${PN}-libgroupsock        = "${libdir}/libgroupsock.so"
FILES_${PN}-libgroupsock-dev    = "${libdir}/libgroupsock.so ${includedir}"

FILES_${PN}-libliveMedia-dbg    = "${libdir}/.debug/libliveMedia.*"
FILES_${PN}-libliveMedia        = "${libdir}/libliveMedia.so"
FILES_${PN}-libliveMedia-dev    = "${libdir}/libliveMedia.so ${includedir}"

FILES_${PN}-libUsageEnvironment-dbg    = "${libdir}/.debug/libUsageEnvironment.*"
FILES_${PN}-libUsageEnvironment        = "${libdir}/libUsageEnvironment.so"
FILES_${PN}-libUsageEnvironment-dev    = "${libdir}/libUsageEnvironment.so ${includedir}"

FILES_${PN} = "${libdir}/lib*.so"
FILES_${PN}-dev = "${libdir}/lib*.so ${includedir}"
FILES_${PN}-dbg = "${libdir}/.debug"
PACKAGES = "${PN} ${PN}-dbg ${PN}-dev"
