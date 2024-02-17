DESCRIPTION = "RTSP proxy (plain TCP to websockets)"
SECTION = "networking"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

DEPENDS += "libwebsockets libuv"

FILESPATH =+ "${WORKSPACE}/vendor/qcom/opensource/qmmf-webserver:"
SRC_URI = "file://rtsp-proxy"
SRCREV = "${AUTOREV}"

CFLAGS += "-Wall -O2 -g"
LDFLAGS += "-lwebsockets -luv"

S = "${WORKDIR}/${PN}"

FILES_${PN} += "rtsp_proxy"

do_compile() {
	oe_runmake
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 rtsp_proxy ${D}${bindir}
}

sysroot_preprocess() {
	install -d ${SYSROOT_DESTDIR}${bindir}
	install -m 0755 ${S}/rtsp_proxy ${SYSROOT_DESTDIR}${bindir}
}

SYSROOT_PREPROCESS_FUNCS += "sysroot_preprocess"
