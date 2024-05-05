SUMMARY = "Python modules and various SELinux utilities."
DESCRIPTION = "\
This package contains Python modules sepolgen, sepolicy; And the \
SELinux utilities audit2allow, chcat, semanage ..."

SECTION = "base"
LICENSE = "GPLv2+"

SRC_URI += "file://fix-sepolicy-install-path.patch \
            file://fix-TypeError-for-seobject.py.patch \
            file://process-ValueError-for-sepolicy-seobject.patch \
"

inherit python-dir

DEPENDS += "python-native libsepol"
RDEPENDS_${BPN}-audit2allow += "\
        python-textutils \
        libselinux-python \
        ${BPN}-sepolgen \
"
RDEPENDS_${BPN}-chcat += "\
        python-codecs \
        python-shell \
        python-stringold \
        python-unixadmin \
        libselinux-python \
        ${BPN} \
"
RDEPENDS_${BPN} += "\
        python-codecs \
        python-io \
        python-ipy \
        python-re \
        python-stringold \
        python-syslog \
        python-unixadmin \
        libselinux-python \
        libsemanage-python \
        setools \
"
RDEPENDS_${BPN}-semanage += "\
        python-core \
        python-ipy \
        python-compression \
        python-xml \
        libselinux-python \
        ${BPN} \
"
RDEPENDS_${BPN}-sepolicy += "\
        python-argparse \
        python-codecs \
        python-core \
        python-syslog \
        ${BPN} \
"
RDEPENDS_${BPN}-sepolgen-ifgen += "\
        python \
        libselinux-python \
"

PACKAGES =+ "\
        ${PN}-audit2allow \
        ${PN}-sepolgen-ifgen \
        ${PN}-chcat \
        ${PN}-semanage \
        ${PN}-sepolgen \
        ${PN}-sepolicy \
"
FILES_${PN}-audit2allow = "\
        ${bindir}/audit2allow \
        ${bindir}/audit2why \
"
FILES_${PN}-chcat = "\
        ${bindir}/chcat \
"
FILES_${PN}-semanage = "\
        ${sbindir}/semanage \
        ${datadir}/bash-completion/completions/semanage \
"
# The ${bindir}/sepolgen is a symlink to ${bindir}/sepolicy
FILES_${PN}-sepolicy += "\
        ${bindir}/sepolgen \
        ${bindir}/sepolicy \
        ${datadir}/bash-completion/completions/sepolicy \
"
FILES_${PN}-sepolgen-ifgen += "\
        ${bindir}/sepolgen-ifgen \
        ${bindir}/sepolgen-ifgen-attr-helper \
"
FILES_${PN}-sepolgen += "\
        ${libdir}/python${PYTHON_BASEVERSION}/site-packages/sepolgen* \
        ${localstatedir}/lib/sepolgen/perm_map \
"
# Map to policycoreutils-python in 2.6
FILES_${PN} += "\
        ${libdir}/python${PYTHON_BASEVERSION}/site-packages/seobject.py* \
        ${libdir}/python${PYTHON_BASEVERSION}/site-packages/sepolicy*.egg-info \
        ${libdir}/python${PYTHON_BASEVERSION}/site-packages/sepolicy/* \
"

EXTRA_OEMAKE += "LIBSEPOLA=${STAGING_LIBDIR}/libsepol.a"
do_install() {
        oe_runmake DESTDIR=${D} \
                LIBDIR="${libdir}" \
                PYTHONLIBDIR='${libdir}/python${PYTHON_BASEVERSION}/site-packages' \
                install
}
