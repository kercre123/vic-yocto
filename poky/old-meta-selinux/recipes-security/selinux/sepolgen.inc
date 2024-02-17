SUMMARY = "Python modules for supporting various SELinux utilities."
DESCRIPTION = "\
This package contains a Python module that forms the core of the \
modern audit2allow (which is a part of the package policycoreutils). \
The sepolgen library is structured to give flexibility to the \
application using it. The library contains: Reference Policy \
Representation, which are Objects for representing policies and the \
reference policy interfaces. Secondly, it has objects and algorithms \
for representing access and sets of access in an abstract way and \
searching that access. It also has a parser for reference policy \
"headers". It contains infrastructure for parsing SELinux related \
messages as produced by the audit system. It has facilities for \
generating policy based on required access."
 
SECTION = "base"
LICENSE = "LGPLv2+"

FILES_${PN} = "${libdir}/python${PYTHON_BASEVERSION}/site-packages/*"

DEPENDS += "python"

FILES_${PN} += "${libdir}/python${PYTHON_BASEVERSION}/site-packages \
		/var/lib/sepolgen"

do_install() {
	oe_runmake DESTDIR=${D} \
			PYTHONLIBDIR='${libdir}/python${PYTHON_BASEVERSION}/site-packages' \
			install
}

BBCLASSEXTEND = "native"

