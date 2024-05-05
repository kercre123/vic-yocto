DESCRIPTION = "A multi-platform support library with a focus on asynchronous I/O"
HOMEPAGE = "http://libuv.org/"
BUGTRACKER = "https://github.com/libuv/libuv/issues"
SECTION = "support"
MAINTAINER = "libuv team"

LICENSE = "MIT & BSD-2-Clause & BSD-3-Clause & ISC"
LIC_FILES_CHKSUM = "file://LICENSE;md5=bb5ea0d651f4c3519327171906045775"

SRC_URI = "https://github.com/libuv/${PN}/archive/v${PV}.tar.gz;downloadfilename=${PN}-${PV}.tar.gz"

inherit autotools

do_configure() {
	${S}/autogen.sh || bbnote "${PN} failed to autogen.sh"
	oe_runconf
}
