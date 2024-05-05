DESCRIPTION = "Start up script for upgrading recovery/recoveryfs partitions"
HOMEPAGE = "http://codeaurora.org"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
LICENSE = "BSD"

SRC_URI +="file://${BASEMACHINE}/trigger-recovery-updater.sh"
SRC_URI +="file://${BASEMACHINE}/recovery-updater.sh"
SRC_URI +="file://recovery_updater.service"

S = "${WORKDIR}/${BASEMACHINE}"

PR = "r4"

inherit systemd
inherit update-rc.d

INITSCRIPT_NAME = "trigger-recovery-updater.sh"
INITSCRIPT_PARAMS = "start 72 S ."
INITSCRIPT_PARAMS_mdm = "start 72 S ."

FILES_${PN} += "${systemd_unitdir}/system/"

do_install() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
       install -m 0755 ${WORKDIR}/${BASEMACHINE}/recovery-updater.sh -D \
           ${D}${sysconfdir}/recovery-updater.sh
       install -m 0755 ${WORKDIR}/${BASEMACHINE}/trigger-recovery-updater.sh -D \
           ${D}${sysconfdir}/initscripts/trigger-recovery-updater.sh
       install -d ${D}${systemd_unitdir}/system/
       install -m 0644 ${WORKDIR}/recovery_updater.service -D \
           ${D}${systemd_unitdir}/system/recovery_updater.service

       # enable the service for multi-user.target
       install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
       ln -sf ${systemd_unitdir}/system/recovery_updater.service \
           ${D}${systemd_unitdir}/system/multi-user.target.wants/recovery_updater.service
    else
       install -m 0744 ${WORKDIR}/${BASEMACHINE}/recovery-updater.sh -D \
           ${D}${sysconfdir}/recovery-updater.sh
       install -m 0744 ${WORKDIR}/${BASEMACHINE}/trigger-recovery-updater.sh -D \
           ${D}${sysconfdir}/init.d/trigger-recovery-updater.sh
    fi
}
