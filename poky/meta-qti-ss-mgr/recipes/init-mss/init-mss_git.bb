inherit autotools-brokensep update-rc.d

DESCRIPTION = "Modem init"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
PR = "r7"

FILESPATH =+ "${WORKSPACE}:"
FILESEXTRAPATHS_prepend := "${THISDIR}/init_mss:"

SRC_URI = "file://mdm-ss-mgr/init_mss/"
SRC_URI += "file://init_sys_mss.service"

S = "${WORKDIR}/mdm-ss-mgr/init_mss/"
EXTRA_OECONF += " ${@bb.utils.contains('BASEMACHINE', 'apq8009', '--enable-indefinite-sleep', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('BASEMACHINE', 'apq8017', '--enable-indefinite-sleep', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('BASEMACHINE', 'apq8053', '--enable-indefinite-sleep', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('BASEMACHINE', 'apq8096', '--enable-indefinite-sleep', '', d)}"
EXTRA_OECONF += " ${@bb.utils.contains('BASEMACHINE', 'apq8098', '--enable-indefinite-sleep', '', d)}"

FILES_${PN} += "${systemd_unitdir}/system/"

INITSCRIPT_NAME = "init_sys_mss"
INITSCRIPT_PARAMS = "start 38 2 3 4 5 ."

do_install() {
    install -m 0755 ${S}/init_mss -D ${D}/sbin/init_mss
    install -m 0755 ${S}/start_mss -D ${D}${sysconfdir}/init.d/init_sys_mss
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -d ${D}${systemd_unitdir}/system/
        install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
        install -d ${D}${systemd_unitdir}/system/ffbm.target.wants/
        install -m 0644 ${WORKDIR}/init_sys_mss.service -D ${D}${systemd_unitdir}/system/init_sys_mss.service
        ln -sf ${systemd_unitdir}/system/init_sys_mss.service \
            ${D}${systemd_unitdir}/system/multi-user.target.wants/init_sys_mss.service
        ln -sf ${systemd_unitdir}/system/init_sys_mss.service \
            ${D}${systemd_unitdir}/system/ffbm.target.wants/init_sys_mss.service
    fi
}
