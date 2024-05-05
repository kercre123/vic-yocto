inherit selinux

DEPENDS += "${LIBSELINUX}"

do_configure_prepend () {
	export LINUX_HASSELINUX="${@target_selinux(d, 'Y', 'N')}"
	export LSOF_CFGF="${CFLAGS}"
	export LSOF_CFGL="${LDFLAGS}"
	export LSOF_CC="${BUILD_CC}"
}

do_compile () {
	oe_runmake 'CC=${CC}' 'DEBUG='
}
