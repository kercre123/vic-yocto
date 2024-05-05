PR = "r21"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

#SRC_URI += "file://do-not-install-unnecessary-udev-rules.patch"
SRC_URI_append_msm8960 += " file://${BASEMACHINE}/local.rules"
SRC_URI_append_msm8974 += "file://${BASEMACHINE}/local.rules \
                           file://${BASEMACHINE}/set-dev-nodes.sh"
SRC_URI_append_msm8610 += "file://${BASEMACHINE}/local.rules \
                           file://${BASEMACHINE}/set-dev-nodes.sh"
SRC_URI_append_msm8226 += "file://${BASEMACHINE}/local.rules \
                           file://${BASEMACHINE}/set-dev-nodes.sh"

do_install_append_msm8974 () {
     install -d ${D}${sysconfdir}/udev/scripts/
     install -m 0755 ${FILESEXTRAPATHS}/${BASEMACHINE}/set-dev-nodes.sh ${D}${sysconfdir}/udev/scripts/set-dev-nodes.sh
}

do_install_append_msm8610 () {
     install -d ${D}${sysconfdir}/udev/scripts/
     install -m 0755 ${FILESEXTRAPATHS}/${BASEMACHINE}/set-dev-nodes.sh ${D}${sysconfdir}/udev/scripts/set-dev-nodes.sh
}

do_install_append_msm8226 () {
     install -d ${D}${sysconfdir}/udev/scripts/
     install -m 0755 ${FILESEXTRAPATHS}/${BASEMACHINE}/set-dev-nodes.sh ${D}${sysconfdir}/udev/scripts/set-dev-nodes.sh
}

do_install_append () {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd-minimal', 'true', 'false', d)}; then
        rm -rf ${D}/lib/udev/rules.d/*
    fi
}
