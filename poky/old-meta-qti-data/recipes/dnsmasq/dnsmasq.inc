DESCRIPTION = "Dnsmasq is a lightweight, easy to configure DNS forwarder and DHCP server."
HOMEPAGE = "http://www.thekelleys.org.uk/dnsmasq/doc.html"
SECTION = "console/network"
LICENSE = "GPL"
LICENSE_${PN} = "GPL"
LIC_FILES_CHKSUM = "file://COPYING;md5=0636e73ff0215e8d672dc4c32c317bb3"

PR = "r3"

#at least versions 2.15 and prior are moved to the archive folder on the server
SRC_URI = "http://www.thekelleys.org.uk/dnsmasq/${@['archive/', ''][float(d.getVar('PV',True).split('.')[1]) > 15]}dnsmasq-${PV}.tar.gz;name=dnsmasq-${PV} \
           file://init \
           file://dnsmasq.conf \
           file://dnsmasq_script.sh\
           file://dnsmasq.service"



do_install () {
        oe_runmake "PREFIX=${D}${prefix}" \
                   "BINDIR=${D}${bindir}" \
                   "MANDIR=${D}${mandir}" \
                   install
        install -d ${D}${sysconfdir}/ ${D}${sysconfdir}/init.d ${D}${sysconfdir}/dnsmasq.d
        install -d ${D}${userfsdatadir}/
        install -m 644 ${WORKDIR}/dnsmasq.conf ${D}${userfsdatadir}
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
         install -d ${D}/etc/initscripts
         install -m 755 ${WORKDIR}/init ${D}${sysconfdir}/initscripts/dnsmasq
         install -d ${D}/etc/systemd/system/
         install -m 0644 ${WORKDIR}/dnsmasq.service -D ${D}/etc/systemd/system/dnsmasq.service
         install -d ${D}/etc/systemd/system/multi-user.target.wants/
         # enable the service for multi-user.target
         ln -sf /etc/systemd/system/dnsmasq.service \
               ${D}/etc/systemd/system/multi-user.target.wants/dnsmasq.service
        else
         install -m 755 ${WORKDIR}/init ${D}${sysconfdir}/init.d/dnsmasq
        fi
        install -d ${D}${base_bindir}
        install -m 0755 ${WORKDIR}/dnsmasq_script.sh ${D}${base_bindir}
}

CONFFILES_${PN} = "${userfsdatadir}/dnsmasq.conf"
FILES_${PN} += "${userfsdatadir}/dnsmasq.conf"
