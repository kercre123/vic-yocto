inherit autotools
DESCRIPTION = "JQUERY UI"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://index.html;md5=6ca9561ffd0c9af85632f0279786c0b0"

PR = "r0"
do_configure() {
  :
}

do_compile() {
  :
}

do_install() {
        install -m 0755 -d ${D}/WEBSERVER/www/js/
        install -m 0755 -d ${D}/WEBSERVER/www/js/images
        install -m 0644 ${WORKDIR}/jquery-ui-${PV}.custom/js/jquery-ui-${PV}.custom.min.js ${D}/WEBSERVER/www/js/jquery-ui.js
        install -m 0644 ${WORKDIR}/jquery-ui-${PV}.custom/css/smoothness/jquery-ui-${PV}.custom.css ${D}/WEBSERVER/www/js/jquery-ui.css
        install -m 0644 ${WORKDIR}/jquery-ui-${PV}.custom/css/smoothness/images/ui-icons_888888_256x240.png ${D}/WEBSERVER/www/js/images/ui-icons_888888_256x240.png
}

SRC_URI = "http://jqueryui.com/resources/download/jquery-ui-${PV}.custom.zip"

S = "${WORKDIR}/jquery-ui-${PV}.custom"

FILES_${PN} += "/WEBSERVER/www/js/jquery-ui.js"
FILES_${PN} += "/WEBSERVER/www/js/jquery-ui.css"
FILES_${PN} += "/WEBSERVER/www/js/images/ui-icons_888888_256x240.png"

SRC_URI[md5sum] = "a3aa1236c1a3a8a9e14f31716a3ec8f7"
SRC_URI[sha256sum] = "069839601aad81e7f0f67e95847980e02aca8158d4135eec3f936a0920574b80"
