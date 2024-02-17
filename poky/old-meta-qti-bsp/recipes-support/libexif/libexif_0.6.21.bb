inherit autotools gettext
SUMMARY = "Library to read the extended image information (EXIF) from JPEG pictures"
HOMEPAGE = "http://sourceforge.net/projects/libexif"
BUGTRACKER = "http://sourceforge.net/tracker/?group_id=12272&atid=112272"
LICENSE = "LGPLv2.1"
PRIORITY = "optional"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/LGPL-2.1;md5=1a6d268fd218675ffea8be556788b780"
FILES_${PN} += "/lib/*"

# Package Revision (update whenever recipe is changed)
PR = "r0"

SRC_URI = "\
    http://downloads.sourceforge.net/project/${PN}/${PN}/${PV}/${PN}-${PV}.tar.gz \
"

SRC_URI[md5sum] = "9321c409a3e588d4a99d63063ef4bbb7"
SRC_URI[sha256sum] = "edb7eb13664cf950a6edd132b75e99afe61c5effe2f16494e6d27bc404b287bf"

EXTRA_OECONF = "--libdir=${base_libdir}"
