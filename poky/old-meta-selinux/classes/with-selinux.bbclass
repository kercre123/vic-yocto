inherit selinux

PACKAGECONFIG_append = " ${@target_selinux(d)}"
PACKAGECONFIG[selinux] = "--with-selinux,--without-selinux,libselinux,"
