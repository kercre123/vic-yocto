SUMMARY = "Policy analysis tools for SELinux"
DESCRIPTION = "\
SETools is a collection of graphical tools, command-line tools, and \
libraries designed to facilitate SELinux policy analysis. \
\n\
This meta-package depends upon the main packages necessary to run \
SETools."
SECTION = "base"
LICENSE = "GPLv2 & LGPLv2.1"

SRC_URI = "https://github.com/TresysTechnology/setools/archive/${PV}.tar.gz;downloadfilename=setools-${PV}.tar.gz \
           file://setools4-fixes-for-cross-compiling.patch \
           file://setools4-fix-cross-compiling-errors-for-powerpc-mips.patch \
           file://Fix-build-failure-with-GCC-7-due-to-possible-truncat.patch \
"

SRC_URI[md5sum] = "54cf5c0ca2aa4ef7c6ac153981af34cd"
SRC_URI[sha256sum] = "46a927ea2b163cbe1d35cc35da43e45853e13720c7e02d4cf75a498783c19610"

LIC_FILES_CHKSUM = "file://${S}/COPYING;md5=83a5eb6974c11f30785e90d0eeccf40c \
                    file://${S}/COPYING.GPL;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
                    file://${S}/COPYING.LGPL;md5=4fbd65380cdd255951079008b364516c"

DEPENDS += "bison-native flex-native swig-native python libsepol"

RDEPENDS_${PN} += "python-networkx python-enum34 python-decorator python-setuptools \
                   python-logging python-json python-argparse libselinux-python"

RPROVIDES_${PN} += "${PN}-console"

inherit setuptools

do_install_append() {
	# Need PyQt5 support, disable gui tools
	rm -f ${D}${bindir}/apol
	rm -rf ${D}${libdir}/${PYTHON_DIR}/site-packages/setoolsgui
}
