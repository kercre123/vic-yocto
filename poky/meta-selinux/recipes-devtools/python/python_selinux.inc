# If selinux enabled, disable handlers to rw command history file
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit selinux

