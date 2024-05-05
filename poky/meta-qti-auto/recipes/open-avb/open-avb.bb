DESCRIPTION = "open-avb tools"
HOMEPAGE = "https://github.com/AVnu/Open-AVB"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://examples/LICENSE;md5=81ccd62d4bc28bafc5e1a2576536b927 \
		    file://daemons/LICENSE;md5=81ccd62d4bc28bafc5e1a2576536b927 \
		    file://lib/avtp_pipeline/LICENSE;md5=8f7b370a91d698ed80d2d20e8e01fbb6"

PR = "r1"

DEPENDS += "alsa-lib alsa-intf libpcap pciutils cmake-native glib-2.0"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://external/open-avb/"

S = "${WORKDIR}/external/open-avb/"

EXTRA_OEMAKE += "AVB_FEATURE_NEUTRINO=1"
EXTRA_OEMAKE_append_mdm9650 = " PCI_SUPPORT_INCLUDED=1"
EXTRA_OEMAKE_append_mdm9607 = " PCI_SUPPORT_INCLUDED=0"

do_compile() {
    oe_runmake daemons_all
    oe_runmake avtp_pipeline
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/daemons/maap/linux/maap_daemon ${D}${bindir}
    install -m 0755 ${S}/daemons/mrpd/mrpd ${D}${bindir}
    install -m 0755 ${S}/daemons/mrpd/mrpctl ${D}${bindir}
    install -m 0755 ${S}/daemons/gptp/linux/build/obj/daemon_cl ${D}${bindir}
    install -m 0755 ${S}/lib/avtp_pipeline/build/bin/openavb_harness ${D}${bindir}
    install -d ${D}${userfsdatadir}/avb
    install -m 0644 ${S}/lib/avtp_pipeline/build/bin/*.ini ${D}${userfsdatadir}/avb
}

FILES_${PN} += "${bindir}/*"
FILES_${PN} += "${userfsdatadir}/avb/*"
