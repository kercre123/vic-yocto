DESCRIPTION = "Node.js is a JavaScript runtime built on Chrome's V8 JavaScript engine"
HOMEPAGE = "http://nodejs.org"

LICENSE = "MIT"

SRC_URI = "https://github.com/nodejs/node/archive/v${PV}.tar.gz"

LIC_FILES_CHKSUM = "file://LICENSE;md5=41a3a0ccf7f515cac3377389dd8faac8"

SRC_URI[md5sum] = "94d87c508a3140005a5df025555b40b6"
SRC_URI[sha256sum] = "a9ecd36dd8133084315a7d5f64a2754f3b750b145ec0c2a13357ff2a4f8a73b3"

S = "${WORKDIR}/node-${PV}"

DEPENDS = "openssl"

PACKAGECONFIG[zlib] = "--shared-zlib,,zlib,"
PACKAGECONFIG[openssl] = "--shared-openssl,,openssl,"
PACKAGECONFIG[v8-inspector] = ",--without-inspector,,"

do_configure() {
	./configure --prefix="${prefix}" \
		    --dest-os=linux \
		    --without-snapshot \
		    --with-intl=none \
	            --shared-openssl \
		    ${EXTRA_OECONF}
}

do_compile() {
	oe_runmake BUILDTYPE=Release
}

do_install() {
	oe_runmake install DESTDIR="${D}"
}

FILES_${PN} =+ "${exec_prefix}/lib/node_modules ${bindir}npm"

BBCLASSEXTEND = "native nativesdk"
