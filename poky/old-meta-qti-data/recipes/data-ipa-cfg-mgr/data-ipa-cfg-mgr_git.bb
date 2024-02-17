inherit autotools-brokensep pkgconfig update-rc.d

DESCRIPTION = "Qualcomm IPA"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r4"

DEPENDS  = "glib-2.0"
DEPENDS += "libxml2"
DEPENDS += "libnetfilter-conntrack"
DEPENDS += "virtual/kernel"

EXTRA_OECONF = "--with-kernel=${STAGING_KERNEL_DIR} \
                --enable-target=${BASEMACHINE} \
                --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include --with-glib"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://data-ipa-cfg-mgr"

S = "${WORKDIR}/data-ipa-cfg-mgr"

INITSCRIPT_NAME   = "start_ipacm_le"
INITSCRIPT_PARAMS = "start 38 S . stop 62 0 1 6 ."
FILES_${PN} += "${userfsdatadir}/misc/ipa/IPACM_cfg.xml"
