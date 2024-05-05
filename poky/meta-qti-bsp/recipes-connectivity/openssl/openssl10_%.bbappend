PROVIDES += "openssl"

PACKAGES =+ "libcrypto libssl openssl-conf openssl-engines openssl-misc "

FILES_libcrypto = "${libdir}/libcrypto${SOLIBS}"
FILES_libssl   = "${libdir}/libssl${SOLIBS}"
FILES_openssl-conf = "${sysconfdir}/ssl/openssl.conf"
FILES_openssl-engines = "${libdir}/ssl/engines/*.so ${libdir}/engines"
FILES_openssl-misc = "${libdir}/ssl/misc"
FILES_openssl =+ "${libdir}/ssl/* "
FILES_openssl_append_class-nativesdk = " ${SDKPATHNATIVE}/environment-setup.d/openssl.sh"

CONFFILES_openssl-conf = "${sysconfdir}/ssl/openssl.cnf"

PACKAGE_PREPROCESS_FUNCS_remove = "openssl_package_preprocess"

RRECOMMENDS_libcrypto += "openssl-conf"

RDEPENDS_openssl-bin = "perl"
RDEPENDS_openssl-misc = "perl"
RDEPENDS_openssl-ptest += "openssl-bin perl perl-modules bash python"


RPROVIDES_${PN} += "openssl-locale-.*"
RPROVIDES_${PN} += "openssl"
RPROVIDES_${PN} += "openssl-dbg"
RPROVIDES_${PN} += "openssl-dev"
RPROVIDES_${PN} += "openssl-doc"
RPROVIDES_${PN} += "openssl-locale"
RPROVIDES_${PN} += "openssl-staticdev"
RPROVIDES_${PN} += "openssl-bin"
