FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI += "\
    file://index.html.lighttpd \
    file://lighttpd \
    file://lighttpd.conf \
    file://lighttpd.user \
    file://openssl.cnf \
    file://lighttpd.service \
"

DEPENDS += " openssl"
RDEPENDS_${PN} += " \
               lighttpd-module-alias \
               lighttpd-module-compress \
               lighttpd-module-cgi \
               lighttpd-module-auth \
               lighttpd-module-redirect \
               lighttpd-module-evasive \
               lighttpd-module-authn-file \
"
EXTRA_OECONF += " \
             --with-openssl \
             --with-openssl-libs=${STAGING_LIBDIR} \
"
do_install_append() {
   install -d ${D}${userfsdatadir}
   install -d ${D}${userfsdatadir}/www
   install -m 0755 ${WORKDIR}/openssl.cnf ${D}${userfsdatadir}
   install -m 0770 ${WORKDIR}/lighttpd.user ${D}${userfsdatadir}/www/lighttpd.user
   rm -rf ${D}${sysconfdir}/lighttpd.conf
   install -m 0755 ${WORKDIR}/lighttpd.conf ${D}${userfsdatadir}
   rm -rf ${D}/www/logs ${D}/www/var
}
FILES_${PN} += "${userfsdatadir}/lighttpd.conf"
FILES_${PN} += "${userfsdatadir}/openssl.cnf"
FILES_${PN} += "${userfsdatadir}/www/*"
