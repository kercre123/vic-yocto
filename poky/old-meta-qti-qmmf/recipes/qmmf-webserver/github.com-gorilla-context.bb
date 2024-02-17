DESCRIPTION = "github.com/gorilla/context"

GO_IMPORT = "github.com/gorilla/context"

inherit go

DEST_DIR :="${PN}-${PV}"
DEST_DIR_remove = "${BBEXTENDVARIANT}-"
SRC_URI = "git://github.com/gorilla/context;protocol=https;destsuffix=${DEST_DIR}/src/${GO_IMPORT}"
SRCREV = "${AUTOREV}"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://src/${GO_IMPORT}/LICENSE;md5=c50f6bd9c1e15ed0bad3bea18e3c1b7f"

FILES_${PN} += "${GOBIN_FINAL}/*"
