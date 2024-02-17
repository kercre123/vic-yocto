
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://mount-var-lib-docker.service"
SRC_URI += "file://mount-systemrw-docker.service"


do_install_append() {
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system
        install -m 644 ${WORKDIR}/mount-var-lib-docker.service \
          ${D}/${systemd_unitdir}/system/
        install -m 644 ${WORKDIR}/mount-systemrw-docker.service \
          ${D}/${systemd_unitdir}/system/

        install -d ${D}${systemd_unitdir}/system/docker.service.wants/
        install -d ${D}${systemd_unitdir}/system/docker.service.requires/
        ln -sf ../mount-var-lib-docker.service \
          ${D}${systemd_unitdir}/system/docker.service.wants/mount-var-lib-docker.service
        ln -sf ../mount-systemrw-docker.service \
          ${D}${systemd_unitdir}/system/docker.service.wants/mount-systemrw-docker.service
        ln -sf ../mount-var-lib-docker.service \
          ${D}${systemd_unitdir}/system/docker.service.requires/mount-var-lib-docker.service
        ln -sf ../mount-systemrw-docker.service \
          ${D}${systemd_unitdir}/system/docker.service.requires/mount-systemrw-docker.service

        install -d ${D}${systemd_unitdir}/system/sysinit.target.wants
        ln -sf ../mount-var-lib-docker.service \
          ${D}${systemd_unitdir}/system/sysinit.target.wants/mount-var-lib-docker.service
        ln -sf ../mount-systemrw-docker.service \
          ${D}${systemd_unitdir}/system/sysinit.target.wants/mount-systemrw-docker.service

        install -d ${D}/var/lib/docker
        install -d ${D}/etc/docker
    fi
}

FILES_${PN} += "${userfsdatadir}/docker"
