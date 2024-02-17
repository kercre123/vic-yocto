DESCRIPTION = "Node.js is a JavaScript runtime built on Chrome's V8 JavaScript engine"
HOMEPAGE = "http://nodejs.org"

LICENSE = "MIT"

SRC_URI = "https://github.com/nodejs/node/archive/v${PV}.tar.gz"

LIC_FILES_CHKSUM = "file://LICENSE;md5=41a3a0ccf7f515cac3377389dd8faac8"

SRC_URI[md5sum] = "67d6844eb8f5c3dc4d36ad4a74577482"
SRC_URI[sha256sum] = "8326fe844a4e33234c319bf16eb2c7ebba9f86898df93d6be0098569ee569af6"

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
