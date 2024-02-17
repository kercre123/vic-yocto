PR .= ".5"

inherit with-selinux

FILESEXTRAPATHS_prepend := "${@target_selinux(d, '${THISDIR}/files:')}"

# There is no distro feature just for audit.  If we want it,
# uncomment the following.
#
#PACKAGECONFIG += "${@target_selinux(d, 'audit')}"

PACKAGECONFIG[audit] = "--with-audit=linux,--without-audit,audit,"

