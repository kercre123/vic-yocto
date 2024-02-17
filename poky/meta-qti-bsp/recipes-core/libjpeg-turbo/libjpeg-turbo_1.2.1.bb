inherit autotools pkgconfig
SUMMARY = "Accelerates basline JPEG compression and decompression."
HOMEPAGE = "http://sourceforge.net/projects/libjpeg-turbo/"
BUGTRACKER = "http://sourceforge.net/p/libjpeg-turbo/bugs/"
LICENSE = "GPLv2"
PRIORITY = "optional"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

# Package Revision (update whenever recipe is changed)
PR = "r2"

SRC_URI = "\
    http://downloads.sourceforge.net/project/${BPN}/${PV}/${BPN}-${PV}.tar.gz \
"

SRC_URI[md5sum] = "f61e60ff01381ece4d2fe65eeb52669e"
SRC_URI[sha256sum] = "cb3323f054a02cedad193bd0ca418d46934447f995d19e678ea64f78e4903770"

# Drop-in replacement for jpeg
PROVIDES = "jpeg"
RPROVIDES_${PN} += "jpeg"
RREPLACES_${PN} += "jpeg"
RCONFLICTS_${PN} += "jpeg"


EXTRA_OECONF = "--with-jpeg8 "

PACKAGES =+ "jpeg-tools libturbojpeg"

DESCRIPTION_jpeg-tools = "The jpeg-tools package includes the client programs for access libjpeg functionality. \
These tools allow for the compression, decompression, transformation and display of JPEG files."
FILES_jpeg-tools =  "${bindir}/*"

FILES_libturbojpeg = "${libdir}/libturbojpeg.so"
INSANE_SKIP_libturbojpeg = "dev-so"

BBCLASSEXTEND = "native"
DEPENDS = "nasm-native"

LEAD_SONAME = "libjpeg.so.8"
#EXTRA_OECONF = "--libdir=${base_libdir}"
