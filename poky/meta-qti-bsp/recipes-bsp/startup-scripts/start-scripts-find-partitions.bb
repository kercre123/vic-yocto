DESCRIPTION = "Start up script for detecting partitions"
HOMEPAGE = "http://codeaurora.org"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
LICENSE = "BSD"

SRC_URI +="file://${BASEMACHINE}/find_partitions.sh"
SRC_URI +="file://find_partitions.service"

S = "${WORKDIR}/${BASEMACHINE}"

PR = "r4"

inherit systemd
inherit update-rc.d

INITSCRIPT_NAME = "find_partitions.sh"
INITSCRIPT_PARAMS = "start 36 S ."
INITSCRIPT_PARAMS_mdm = "start 30 S ."

FILES_${PN} += "${systemd_unitdir}/system/"

do_install() {
     if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
       install -m 0755 ${WORKDIR}/${BASEMACHINE}/find_partitions.sh -D ${D}${sysconfdir}/initscripts/find_partitions.sh
       install -d  ${D}${systemd_unitdir}/system/
       install -m 0644 ${WORKDIR}/find_partitions.service -D ${D}${systemd_unitdir}/system/find_partitions.service
       install -d ${D}${systemd_unitdir}/system/sysinit.target.wants/
       # enable the service for sysinit.target
       ln -sf ${systemd_unitdir}/system/find_partitions.service \
            ${D}${systemd_unitdir}/system/sysinit.target.wants/find_partitions.service
    else
       install -m 0755 ${WORKDIR}/${BASEMACHINE}/find_partitions.sh -D ${D}${sysconfdir}/init.d/find_partitions.sh
    fi
}

