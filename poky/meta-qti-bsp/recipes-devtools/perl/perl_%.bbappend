require perl-rdepends_${PV}.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI += "\
        file://makedepend.SH.patch \
        "
