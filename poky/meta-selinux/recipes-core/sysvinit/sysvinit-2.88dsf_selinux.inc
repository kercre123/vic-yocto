FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

B = "${S}"

SRC_URI += "file://sysvinit-fix-is_selinux_enabled.patch"

inherit selinux

DEPENDS += "${LIBSELINUX}"

EXTRA_OEMAKE += "${@target_selinux(d, 'WITH_SELINUX=\"yes\"')}"
