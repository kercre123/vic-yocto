do_install_append () {
	if [ ! ${D}${libdir} -ef ${D}${base_libdir} -a -e ${D}${libdir}/libpcre.so ]; then
		realsofile=`readlink ${D}${libdir}/libpcre.so`
		mkdir -p ${D}/${base_libdir}/
		mv -f ${D}${libdir}/libpcre.so.* ${D}${base_libdir}/
		relpath=${@os.path.relpath("${base_libdir}", "${libdir}")}
		ln -sf ${relpath}/${realsofile} ${D}${libdir}/libpcre.so
		ln -sf ${relpath}/${realsofile} ${D}${libdir}/libpcre.so.1
	fi
}

FILES_${PN} += "${base_libdir}/libpcre.so.*"
