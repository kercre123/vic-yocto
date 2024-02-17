HOMEPAGE = "https://github.com/SELinuxProject"

# EXTRA_OEMAKE is typically: -e MAKEFLAGS=
# "MAKEFLAGS= " causes problems as ENV variables will not pass to subdirs, so
# we redefine EXTRA_OEMAKE here
EXTRA_OEMAKE = "-e"

do_compile() {
    oe_runmake all \
            INCLUDEDIR='${STAGING_INCDIR}' \
            LIBDIR='${STAGING_LIBDIR}'
}

do_install() {
    oe_runmake install \
            DESTDIR="${D}" \
            PREFIX="${D}/${prefix}" \
            INCLUDEDIR="${D}/${includedir}" \
            LIBDIR="${D}/${libdir}" \
            SHLIBDIR="${D}/${base_libdir}"
}
