PR .= ".1"

inherit with-selinux with-audit

PACKAGECONFIG[selinux] = "--with-selinux,--without-selinux,libselinux libsemanage,"

FILESEXTRAPATHS_prepend := "${@target_selinux(d, '${THISDIR}/files:')}"
