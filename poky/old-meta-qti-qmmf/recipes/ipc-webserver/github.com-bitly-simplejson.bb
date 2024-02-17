DESCRIPTION = "github.com/bitly/go-simplejson"

GO_IMPORT = "github.com/bitly/go-simplejson"

inherit go

DEST_DIR :="${PN}-${PV}"
DEST_DIR_remove = "${BBEXTENDVARIANT}-"
SRC_URI = "git://github.com/bitly/go-simplejson;protocol=https;destsuffix=${DEST_DIR}/src/${GO_IMPORT}"
SRCREV = "${AUTOREV}"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://src/${GO_IMPORT}/LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"

FILES_${PN} += "${GOBIN_FINAL}/*"
