FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://rpm-fix-build-bug.patch \
	   "

FILES_${PN} += "${libdir}/rpm/bin/spooktool \
                ${libdir}/rpm/bin/semodule \
               "

inherit with-selinux
PACKAGECONFIG[selinux] = "${WITH_SELINUX},${WITHOUT_SELINUX},libsemanage,"
