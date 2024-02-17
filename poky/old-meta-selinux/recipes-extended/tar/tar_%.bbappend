FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

inherit with-selinux


PACKAGECONFIG += "${@bb.utils.contains('DISTRO_FEATURES', 'acl', 'acl', '', d)}"
