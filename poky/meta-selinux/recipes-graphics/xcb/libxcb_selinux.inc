inherit enable-selinux
# libxcb-xselinux will not build with libselinux, so remove the depend
PACKAGECONFIG[selinux] = "--enable-selinux,--disable-selinux,,"

PACKAGES += "${PN}-xselinux"
FILES_${PN}-xselinux += "${libdir}/libxcb-xselinux.so.*"
