inherit selinux
# If selinux enabled, disable handlers to rw command history file
FILESEXTRAPATHS_prepend := "${@target_selinux(d, '${THISDIR}/${PN}:')}"
