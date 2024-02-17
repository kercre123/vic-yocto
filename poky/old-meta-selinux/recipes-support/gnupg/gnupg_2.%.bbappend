inherit enable-selinux
# gnupg will not build with libselinux, so remove the depend
PACKAGECONFIG[selinux] = "--enable-selinux-support,--disable-selinux-support,,"
