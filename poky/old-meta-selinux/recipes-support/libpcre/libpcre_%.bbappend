PR .= "9"

do_install_append () {
	if [ ! ${D}${libdir} -ef ${D}${base_libdir} ]; then
		realsofile=`readlink ${D}${libdir}/libpcre.so`
		mkdir -p ${D}/${base_libdir}/
		mv -f ${D}${libdir}/libpcre.so.* ${D}${base_libdir}/
		relpath=${@os.path.relpath("${base_libdir}", "${libdir}")}
		ln -sf ${relpath}/${realsofile} ${D}${libdir}/libpcre.so
	fi
}

FILES_${PN} += "${base_libdir}/libpcre.so.*"
