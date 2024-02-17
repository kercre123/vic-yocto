SUMMARY = "SELinux library and simple utilities"
DESCRIPTION = "libselinux provides an API for SELinux applications to get and set \
process and file security contexts and to obtain security policy \
decisions.  Required for any applications that use the SELinux API."
SECTION = "base"
LICENSE = "PD"

inherit lib_package pythonnative

DEPENDS += "libsepol python libpcre swig-native"
DEPENDS_append_libc-musl = " fts"
RDEPENDS_${PN}-python += "python-core"

PACKAGES += "${PN}-python"
FILES_${PN}-python = "${libdir}/python${PYTHON_BASEVERSION}/site-packages/*"
FILES_${PN}-dbg += "${libdir}/python${PYTHON_BASEVERSION}/site-packages/selinux/.debug/*"

def get_policyconfigarch(d):
    import re
    target = d.getVar('TARGET_ARCH', True)
    p = re.compile('i.86')
    target = p.sub('i386',target)
    return "ARCH=%s" % (target)
EXTRA_OEMAKE += "${@get_policyconfigarch(d)}"

EXTRA_OEMAKE += "LDFLAGS='${LDFLAGS} -lpcre' LIBSEPOLA='${STAGING_LIBDIR}/libsepol.a'"
EXTRA_OEMAKE_append_libc-musl = " FTS_LDLIBS=-lfts"

do_compile_append() {
    oe_runmake pywrap -j1 \
            INCLUDEDIR='${STAGING_INCDIR}' \
            LIBDIR='${STAGING_LIBDIR}' \
            PYINC='-I${STAGING_INCDIR}/python${PYTHON_BASEVERSION}'
}

do_install_append() {
    oe_runmake install-pywrap swigify \
            PYTHONLIBDIR=${D}${libdir}/python${PYTHON_BASEVERSION}/site-packages
    if ! ${@bb.utils.contains('DISTRO_FEATURES','usrmerge','true','false',d)}; then
        rm -rf ${D}${base_sbindir}
    fi
}

BBCLASSEXTEND = "native"
