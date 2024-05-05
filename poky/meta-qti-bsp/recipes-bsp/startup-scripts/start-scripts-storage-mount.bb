DESCRIPTION = "Start up script for partitions mount"
HOMEPAGE = "http://codeaurora.org"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
LICENSE = "BSD"

SRC_URI +="file://${BASEMACHINE}/storage-mount.sh"
SRC_URI +="file://storage-mount.service"

S = "${WORKDIR}/${BASEMACHINE}"

PR = "r5"

inherit systemd update-rc.d

INITSCRIPT_NAME   = "storage-mount.sh"
INITSCRIPT_PARAMS = "start 37 S ."

do_install() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
       install -m 0755 ${WORKDIR}/${BASEMACHINE}/storage-mount.sh -D ${D}${sysconfdir}/initscripts/storage-mount.sh
       install -d ${D}${systemd_unitdir}/system/
       install -m 0644 ${WORKDIR}/storage-mount.service -D ${D}${systemd_unitdir}/system/storage-mount.service
       install -d ${D}${systemd_unitdir}/system/local-fs.target.requires/
       # enable the service for sysinit.target
       ln -sf ${systemd_unitdir}/system/storage-mount.service \
            ${D}${systemd_unitdir}/system/local-fs.target.requires/storage-mount.service
    else
       install -m 0755 ${WORKDIR}/${BASEMACHINE}/storage-mount.sh -D ${D}${sysconfdir}/init.d/storage-mount.sh
    fi
}


pkg_postinst_${PN} () {
        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
         update-alternatives --install ${sysconfdir}/init.d/storage-mount.sh storage-mount.sh storage-mount.sh 60
         [ -n "$D" ] && OPT="-r $D" || OPT="-s"
         # remove all rc.d-links potentially created from alternatives
         update-rc.d $OPT -f storage-mount.sh remove
         update-rc.d $OPT storage-mount.sh multiuser
       fi
}

FILES_${PN} += "${systemd_unitdir}/system/"
