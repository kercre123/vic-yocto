inherit update-rc.d systemd

DESCRIPTION = "Kernel Module Loader"
PR = "r0"

LICENSE          = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=550794465ba0ec5312d6919e203a55f9"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI += "file://evdev_load.service"
SRC_URI += "file://evdev_load.sh"

INITSCRIPT_NAME = "evdev_load.sh"
INITSCRIPT_PARAMS = "start 08 S . stop 60 0 1 6 ."

do_install () {

    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
      # Place mount_event_driver.service in systemd unitdir
      install -d ${D}/etc/systemd/system/
      install -m 0644 ${WORKDIR}/evdev_load.service  \
          -D ${D}/etc/systemd/system/evdev_load.service

      if ${@bb.utils.contains_any('DISTRO_NAME', 'auto nad-core', 'true', 'false', d)}; then
        # Enable the service for sockets.target
        install -d ${D}/etc/systemd/system/sockets.target.wants/
        ln -sf ${sysconfdir}/systemd/system/evdev_load.service \
             ${D}/etc/systemd/system/sockets.target.wants/evdev_load.service
      else
        # Enable the service for multi-user.target
        install -d ${D}/etc/systemd/system/multi-user.target.wants/
        ln -sf ${sysconfdir}/systemd/system/evdev_load.service \
             ${D}/etc/systemd/system/multi-user.target.wants/evdev_load.service
      fi
    else
      install -d ${D}/etc/init.d/
      install -m 0755 ${WORKDIR}/${INITSCRIPT_NAME} \
          -D ${D}/etc/init.d/${INITSCRIPT_NAME}
    fi
}

FILES_${PN} += "/etc/systemd/system/*"
