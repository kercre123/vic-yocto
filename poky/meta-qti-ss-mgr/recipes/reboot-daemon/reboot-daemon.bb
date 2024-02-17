inherit autotools-brokensep

DESCRIPTION = "Rebooter daemon"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=3775480a712fc46a69647678acb234cb"
PR = "r4"

FILESPATH =+ "${WORKSPACE}/mdm-ss-mgr:"

SRC_URI = "file://reboot-daemon"
SRC_URI += "file://reboot-daemon.service"

S = "${WORKDIR}/reboot-daemon"

EXTRA_OEMAKE_append = " CROSS=${HOST_PREFIX}"
FILES_${PN} += "${systemd_unitdir}/system/"

do_install() {
    install -m 0755 ${S}/reboot-daemon -D ${D}/sbin/reboot-daemon
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
      install -d ${D}${systemd_unitdir}/system/
      install -m 0644 ${WORKDIR}/reboot-daemon.service -D ${D}${systemd_unitdir}/system/reboot-daemon.service
      install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
      install -d ${D}${systemd_unitdir}/system/ffbm.target.wants/
      # enable the service for multi-user.target
      ln -sf ${systemd_unitdir}/system/reboot-daemon.service \
           ${D}${systemd_unitdir}/system/multi-user.target.wants/reboot-daemon.service
      ln -sf ${systemd_unitdir}/system/reboot-daemon.service \
           ${D}${systemd_unitdir}/system/ffbm.target.wants/reboot-daemon.service
   fi
}
