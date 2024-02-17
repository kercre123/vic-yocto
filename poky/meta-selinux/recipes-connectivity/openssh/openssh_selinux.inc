inherit with-selinux

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# There is no distro feature just for audit.
PACKAGECONFIG_append = " audit"

PACKAGECONFIG[audit] = "--with-audit=linux,--without-audit,audit,"

