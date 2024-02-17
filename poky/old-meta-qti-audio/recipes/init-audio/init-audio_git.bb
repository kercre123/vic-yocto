inherit autotools update-rc.d systemd

DESCRIPTION = "Installing audio init script"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
PR = "r5"

DEPENDS_append_mdm9635 +="alsa-intf"

SRC_URI = "file://init_qcom_audio"
SRC_URI += "file://init_audio.service"
SRC_URI_msm8974 = "file://${BASEMACHINE}/init_qcom_audio"
SRC_URI_msm8610 = "file://${BASEMACHINE}/init_qcom_audio"

S = "${WORKDIR}"
S_msm8974 = "${WORKDIR}/${BASEMACHINE}"
S_msm8610 = "${WORKDIR}/${BASEMACHINE}"

INITSCRIPT_NAME = "init_qcom_audio"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 1 0 1 6 ."
INITSCRIPT_NAME_apq8009 = "init_qcom_audio"
INITSCRIPT_PARAMS_apq8009 = "start 38 2 3 4 5 . stop 1 0 1 6 ."
INITSCRIPT_NAME_msm8974 = "init_qcom_audio"
INITSCRIPT_PARAMS_msm8974 = "start 99 2 3 4 5 . stop 1 0 1 6 ."
INITSCRIPT_NAME_msm8610 = "init_qcom_audio"
INITSCRIPT_PARAMS_msm8610 = "start 99 2 3 4 5 . stop 1 0 1 6 ."

do_install() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -m 0755 ${S}/init_qcom_audio -D ${D}${sysconfdir}/initscripts/init_qcom_audio
        install -d ${D}/etc/systemd/system/
        install -m 0644 ${S}/init_audio.service -D ${D}${sysconfdir}/systemd/system/init_audio.service
        install -d ${D}/etc/systemd/system/multi-user.target.wants
        ln -sf /etc/systemd/system/init_audio.service ${D}/etc/systemd/system/multi-user.target.wants/init_audio.service
        install -d ${D}/etc/systemd/system/ffbm.target.wants
        ln -sf /etc/systemd/system/init_audio.service ${D}/etc/systemd/system/ffbm.target.wants/init_audio.service
    else
        install -m 0755 ${S}/init_qcom_audio -D ${D}${sysconfdir}/init.d/init_qcom_audio
    fi

}
