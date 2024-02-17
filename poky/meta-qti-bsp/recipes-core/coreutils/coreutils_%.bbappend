do_install_append_mdm(){
	mv ${D}${base_bindir}/cp.coreutils ${D}/cp.coreutils;
	mv ${D}${bindir}/chcon ${D}/chcon;
	rm -rf ${D}${base_bindir};
	rm -rf ${D}/usr;
	install -d ${D}${base_bindir};
	install -d ${D}${bindir};
	mv ${D}/cp.coreutils ${D}${base_bindir}/cp.coreutils;
	mv ${D}/chcon ${D}${bindir}/chcon;
}

do_install_append_sa2150p-nand(){
        mv ${D}${base_bindir}/cp.coreutils ${D}/cp.coreutils;
        mv ${D}${bindir}/chcon ${D}/chcon;
        rm -rf ${D}${base_bindir};
        rm -rf ${D}/usr;
        install -d ${D}${base_bindir};
        install -d ${D}${bindir};
        mv ${D}/cp.coreutils ${D}${base_bindir}/cp.coreutils;
        mv ${D}/chcon ${D}${bindir}/chcon;
}

do_install_append(){
	if ${@bb.utils.contains('DISTRO_FEATURES', 'no-test-bundle', 'true', 'false', d)}; then
           mv ${D}${bindir}/chcon  ${D}/chcon;
           rm -rf ${D}/usr;
           install -d ${D}${bindir};
           mv ${D}/chcon ${D}${bindir}/chcon;
	fi
}
FILES_${PN}_qcs403-som2 = "${base_bindir}/*  ${bindir}/chcon.coreutils"
