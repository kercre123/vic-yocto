inherit autotools
SUMMARY = "SQLite is a software library that implements a self-contained, serverless, zero-configuration, \
transactional SQL database engine."
HOMEPAGE = "http://www.sqlite.org/index.html"
BUGTRACKER = "http://sourceforge.net/tracker/?group_id=243163&atid=1121516&source=navbar"
LICENSE = "GPLv2"
PRIORITY = "optional"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"
DEPENDS = "readline"

# Package Revision (update whenever recipe is changed)
PR = "r1"

SRC_URI = "\
    http://www.sqlite.org/${PN}-${PV}.tar.gz \
"

SRC_URI[md5sum] = "bcb0ab0b5b30116b2531cfeef3c861b4"
SRC_URI[sha256sum] = "782d16b797f6ca879f6f679ba3fb6ceb54bcb0cab65feef332058bf04b36ba8c"
