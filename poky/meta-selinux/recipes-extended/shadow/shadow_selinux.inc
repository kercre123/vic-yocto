FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit with-selinux with-audit

PACKAGECONFIG[selinux] = "--with-selinux,--without-selinux,libselinux libsemanage,"

