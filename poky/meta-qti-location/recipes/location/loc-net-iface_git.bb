inherit autotools-brokensep pkgconfig

DESCRIPTION = "GPS Loc Net Iface"
PR = "r5"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://qcom-opensource/location/loc_net_iface/"
S = "${WORKDIR}/qcom-opensource/location/loc_net_iface"
DEPENDS = "glib-2.0 gps-utils qmi qmi-framework loc-pla data data-items loc-hal"
EXTRA_OECONF = "--with-glib"

PACKAGES = "${PN}"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
FILES_${PN} = "${libdir}/*"
INSANE_SKIP_${PN} = "dev-so"
