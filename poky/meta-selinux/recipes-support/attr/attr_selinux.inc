inherit selinux

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://fix-ptest-failures-when-selinux-enabled.patch"
