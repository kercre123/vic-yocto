FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI += "\
           file://Enable-ffbm.patch \
           file://init-setrlimit-to-enable-coredump.patch \
"
SRC_URI_append_apq8053 += "\
           file://0001-sysvinit-logs-disabled.patch \
"
SRC_URI_append += "${@bb.utils.contains('DISTRO_FEATURES','ro-rootfs','file://ro/rcS-default','file://rcS-default',d)}"

do_install_append() {
  install -d ${D}/firmware
  if ${@bb.utils.contains('DISTRO_FEATURES','ro-rootfs','true','false',d)}; then
   install -m 0644 ${WORKDIR}/ro/rcS-default ${D}${sysconfdir}/default/rcS
  else
   install -m 0644 ${WORKDIR}/rcS-default ${D}${sysconfdir}/default/rcS
  fi
}

FILES_${PN} += "/firmware"
