# List of packages installed onto the root file system as specified by the user.
include ${BASEMACHINE}/${BASEMACHINE}-qsap-image.inc

require include/mdm-bootimg.inc

require include/mdm-ota-target-image-ubi.inc
require include/mdm-ota-target-image-ext4.inc

inherit core-image

MULTILIBRE_ALLOW_REP =. "/usr/include/python2.7/*|${base_bindir}|${base_sbindir}|${bindir}|${sbindir}|${libexecdir}|${sysconfdir}|${nonarch_base_libdir}/udev|/lib/modules/[^/]*/modules.*|"
