inherit go

DESCRIPTION = "IPC Webserver"
SECTION = "networking"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

DEPENDS := "go-cross"
DEPENDS += "github.com-gorilla-muxer"
DEPENDS += "github.com-gorilla-websocket"
DEPENDS += "github.com-bitly-simplejson"
DEPENDS += "libcutils"

FILESPATH =+ "${WORKSPACE}/vendor/qcom/opensource/qmmf-webserver:"
SRC_URI = "file://ipc-webserver"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/ipc-webserver"

FILES_${PN} += "ipc-webserver"

export CGO_ENABLED = "1"
export GOPATH="${S}:${STAGING_LIBDIR}/${TARGET_SYS}/go"

do_compile() {
  go build ipc-webserver.go
}

do_install() {
  install -d ${D}/data/misc/qmmf/ipc_webserver
  install -d ${D}/data/misc/qmmf/ipc_webserver/channel1
  install -d ${D}/data/misc/qmmf/ipc_webserver/channel2
  install -d ${D}/data/misc/qmmf/ipc_webserver/channel3
  install -d ${D}/data/misc/qmmf/ipc_webserver/vam
  install -d ${D}/data/misc/qmmf/ipc_webserver/image
  install -d ${D}/data/misc/qmmf/ipc_webserver/video
  install -d ${D}/${bindir}

  install -m 0755 ipc-webserver ${D}/${bindir}
  install -m 0444 ${S}/res_config ${D}/data/misc/qmmf/ipc_webserver
  install -m 0444 ${S}/net_config ${D}/data/misc/qmmf/ipc_webserver
}

sysroot_preprocess() {
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/channel1
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/channel2
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/channel3
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/vam
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/image
  install -d ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver/video
  install -d ${SYSROOT_DESTDIR}/${bindir}

  install -m 0755 ${S}/ipc-webserver ${SYSROOT_DESTDIR}/${bindir}
  install -m 0444 ${S}/res_config ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver
  install -m 0444 ${S}/net_config ${SYSROOT_DESTDIR}/data/misc/qmmf/ipc_webserver
}

SYSROOT_PREPROCESS_FUNCS += "sysroot_preprocess"

FILES_${PN} += "/data/misc/qmmf/ipc_webserver/*"
