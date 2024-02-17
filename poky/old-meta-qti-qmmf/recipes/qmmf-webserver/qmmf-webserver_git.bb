inherit go

DESCRIPTION = "QMMF Webserver"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

DEPENDS := "go-cross"
DEPENDS += "github.com-gorilla-muxer"
DEPENDS += "qmmf-support"

FILESPATH =+ "${WORKSPACE}/vendor/qcom/opensource/:"
SRC_URI  := "file://qmmf-webserver"
SRC_URI  += "file://qmmf-webserver.service"
SRC_URI  += "file://0001-qmmf-webserver-update-http-library-path.patch"
S = "${WORKDIR}/qmmf-webserver"


export CGO_ENABLED = "1"
export GOPATH="${S}:${STAGING_LIBDIR}/${TARGET_SYS}/go"

do_compile() {
  go build qmmf-webserver.go
}

do_install() {
  install -d "${D}/${bindir}"
  install -m 0755 "${S}/qmmf-webserver" "${D}/${bindir}"
  if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
      install -d ${D}/etc/systemd/system/
      install -m 0644 ${WORKDIR}/qmmf-webserver.service -D ${D}/etc/systemd/system/qmmf-webserver.service
      install -d ${D}/etc/systemd/system/multi-user.target.wants/
      # enable the service for multi-user.target
      ln -sf /etc/systemd/qmmf-webserver.service \
          ${D}/etc/systemd/system/multi-user.target.wants/qmmf-webserver.service
  fi
  # Location for user configs
  install -d ${D}/${userfsdatadir}/misc/qmmf
}

FILES_${PN} = "${bindir}/qmmf-webserver"
FILES_${PN} += "/etc/systemd/system/"
FILES_${PN} += "${userfsdatadir}/*"

# Avoid QA Issue: No GNU_HASH in the elf binary
INSANE_SKIP_${PN} = "ldflags"
