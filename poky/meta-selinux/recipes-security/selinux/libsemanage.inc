SUMMARY = "SELinux binary policy manipulation library"
DESCRIPTION = "libsemanage provides an API for the manipulation of SELinux binary policies. \
It is used by checkpolicy (the policy compiler) and similar tools, as well \
as by programs like load_policy that need to perform specific transformations \
on binary policies such as customizing policy boolean settings."
SECTION = "base"
LICENSE = "LGPLv2.1+"

inherit lib_package python-dir

DEPENDS += "libsepol libselinux bzip2 python bison-native flex-native swig-native"
DEPENDS_append_class-target += "audit"

PACKAGES =+ "${PN}-python"

# For /usr/libexec/selinux/semanage_migrate_store
RDEPENDS_${PN}-python += "python"

FILES_${PN}-python = "${libdir}/python${PYTHON_BASEVERSION}/site-packages/* \
                      ${libexecdir}/selinux/semanage_migrate_store"
FILES_${PN}-dbg += "${libdir}/python${PYTHON_BASEVERSION}/site-packages/.debug/*"

EXTRA_OEMAKE_class-native += "DISABLE_AUDIT=y"

do_compile_append() {
    oe_runmake pywrap \
            INCLUDEDIR='${STAGING_INCDIR}' \
            LIBDIR='${STAGING_LIBDIR}' \
            PYLIBVER='python${PYTHON_BASEVERSION}' \
            PYINC='-I${STAGING_INCDIR}/$(PYLIBVER)' \
            PYLIB='-L${STAGING_LIBDIR}/$(PYLIBVER) -l$(PYLIBVER)' \
            PYTHONLIBDIR='${PYLIB}'
}

do_install_append() {
    oe_runmake install-pywrap swigify \
            PYCEXT='.so' \
            PYTHONLIBDIR='${D}${libdir}/python${PYTHON_BASEVERSION}/site-packages' \
            PYLIBVER='python${PYTHON_BASEVERSION}' \
            PYLIBDIR='${D}/${libdir}/$(PYLIBVER)'

    # Update "policy-version" for semanage.conf
    sed -i 's/^#\s*\(policy-version\s*=\).*$/\1 30/' \
	${D}/etc/selinux/semanage.conf
}

BBCLASSEXTEND = "native"
