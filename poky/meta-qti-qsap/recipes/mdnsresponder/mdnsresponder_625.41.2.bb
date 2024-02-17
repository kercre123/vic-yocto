SUMMARY = "Zeroconf service discovery responder"
HOMEPAGE = "https://github.com/applesrc/mDNSResponder"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${B}/LICENSE;md5=31c50371921e0fb731003bbc665f29bf"

SRC_URI = "git://github.com/applesrc/mDNSResponder.git;destsuffix=mDNSResponder-${PV}"
SRC_URI += "\
           file://001-owrt_compile.patch \
           file://100-oe_compile.patch \
           file://200-Fix_permissions_for_android_kernel.patch \
           file://mdnsd.service \
           file://mDNSResponder.conf \
           file://mdnsresponder.service \
           "

SRCREV = "6460a54768f8e379ff1a3257fcd7b6f55c1c278b"

inherit systemd useradd

PARALLEL_MAKE = ""

S = "${WORKDIR}/mDNSResponder-${PV}"

EXTRA_OEMAKE = " \
    CC="${CC}" \
    LD="${CCLD}" \
    ST="${STRIP}" \
    CFLAGS_EXTRA="${TARGET_CFLAGS}" \
    LINKOPTS_EXTRA="${TARGET_LDFLAGS}" \
    "

USERADD_PACKAGES = "${PN}"
USERADD_PARAM_${PN} = "--system --home-dir /nonexistent --shell /bin/false --groups inet mdns"

do_configure() {
}

do_compile() {
    oe_runmake -C mDNSPosix os=linux DEBUG=0
}

do_install () {
    install -d ${D}${sbindir}
    install -m 0755 mDNSPosix/build/prod/mdnsd ${D}${sbindir}

    install -d ${D}${libdir}
    oe_libinstall -C mDNSPosix/build/prod -so libnss_mdns-0.2 ${D}${libdir}
    ln -s libnss_mdns-0.2.so ${D}${libdir}/libnss_mdns.so.2

    install -d ${D}${libdir}
    cp mDNSPosix/build/prod/libdns_sd.so ${D}${libdir}

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/*.service -D ${D}${systemd_system_unitdir}

    install -d ${D}${includedir}
    install -m 0644 mDNSShared/dns_sd.h ${D}${includedir}

    install -d ${D}${mandir}/man8
    install -m 0644 mDNSShared/mDNSResponder.8 ${D}${mandir}/man8/mdnsd.8

    install -d ${D}${bindir}
    install -m 0755  Clients/build/dns-sd ${D}${bindir}

    install -d ${D}${sysconfdir}
    install -m 0644 mDNSPosix/nss_mdns.conf ${D}${sysconfdir}

    install -d ${D}${mandir}/man5
    install -m 0644 mDNSPosix/nss_mdns.conf.5 ${D}${mandir}/man5

    install -d ${D}${mandir}/man8
    install -m 0644 mDNSPosix/libnss_mdns.8 ${D}${mandir}/man8

    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/mDNSResponder.conf ${D}${sysconfdir}/

    cp mDNSPosix/build/prod/mDNSClientPosix ${D}${bindir}/mDNSClient
    cp mDNSPosix/build/prod/mDNSIdentify ${D}${bindir}/mDNSIdentify
    cp mDNSPosix/build/prod/mDNSNetMonitor ${D}${bindir}/mDNSNetMonitor
    cp mDNSPosix/build/prod/mDNSProxyResponderPosix ${D}${bindir}/mDNSProxyResponder
    cp mDNSPosix/build/prod/mDNSResponderPosix ${D}${bindir}/mDNSResponder

}


FILES_SOLIBSDEV = ""

FILES_mdnsd = "${sbindir}/mdnsd \
               ${libdir}/libnss_mdns-0.2.so \
               ${sysconfdir}/nss_mdns.conf"

FILES_${PN} += "${libdir}/libdns_sd.so \
                ${bindir}/dns-sd \
                ${systemd_system_unitdir} \
                ${bindir}/mDNSClient \
                ${bindir}/mDNSIdentify \
                ${bindir}/mDNSNetMonitor \
                ${bindir}/mDNSProxyResponder \
                ${bindir}/mDNSResponder \
                ${sysconfdir}/mDNSResponder.conf"

FILES_${PN}-dev += "${includedir}/dns_sd.h"

FILES_${PN}-man += "${mandir}/man8/mdnsd.8 \
                    ${mandir}/man5/nss_mdns.conf.5 \
                    ${mandir}/man8/libnss_mdns.8"

PACKAGES = "mdnsd ${PN}-dev ${PN} ${PN}-man ${PN}-dbg"

SYSTEMD_SERVICE_${PN} = "mdnsresponder.service"
SYSTEMD_SERVICE_${PN} += "mdnsd.service"
