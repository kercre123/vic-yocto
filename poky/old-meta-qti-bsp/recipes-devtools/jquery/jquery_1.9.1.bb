inherit autotools
DESCRIPTION = "JQUERY"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://jquery-${PV}.min.js;md5=397754ba49e9e0cf4e7c190da78dda05"

PR = "r0"
do_configure() {
  :
}

do_compile() {
  :
}

do_install() {
        install -m 0755 -d ${D}/WEBSERVER/www/js/
        install -m 0644 ${WORKDIR}/jquery-${PV}.min.js ${D}/WEBSERVER/www/js/jquery.js
}

SRC_URI = "http://code.jquery.com/jquery-1.9.1.min.js"

S = "${WORKDIR}"

FILES_${PN} += "/WEBSERVER/www/js/jquery.js"

SRC_URI[md5sum] = "397754ba49e9e0cf4e7c190da78dda05"
SRC_URI[sha256sum] = "c12f6098e641aaca96c60215800f18f5671039aecf812217fab3c0d152f6adb4"
