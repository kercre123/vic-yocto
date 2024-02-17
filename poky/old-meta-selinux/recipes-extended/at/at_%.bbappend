PR .= ".2"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://at-3.1.13-selinux.patch"

inherit with-selinux
