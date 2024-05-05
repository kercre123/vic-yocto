FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://avahi-daemon.conf"

USERADD_PARAM_avahi-daemon = "--system --home /var/run/avahi-daemon \
                              --no-create-home --shell /bin/false \
                              --groups inet --user-group avahi"

do_install_append() {
  if ${@bb.utils.contains('DISTRO_FEATURES', 'avahi', 'true', 'false', d)}; then
      install -m 644 ${WORKDIR}/avahi-daemon.conf ${D}${sysconfdir}/avahi/avahi-daemon.conf
  fi
}
