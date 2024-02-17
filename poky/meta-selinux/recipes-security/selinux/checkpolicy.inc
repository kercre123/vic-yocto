SUMMARY = "SELinux policy compiler"
DESCRIPTION = "\
This package contains checkpolicy, the SELinux policy compiler. Only \
required for building policies. It uses libsepol to generate the \
binary policy. checkpolicy uses the static libsepol since it deals \
with low level details of the policy that have not been \
encapsulated/abstracted by a proper shared library interface."

SECTION = "base"
LICENSE = "GPLv2+"

DEPENDS += "libsepol bison-native flex-native"

EXTRA_OEMAKE += "LEX='flex'"
EXTRA_OEMAKE += "LIBSEPOLA=${STAGING_LIBDIR}/libsepol.a"

do_install_append() {
	install test/dismod ${D}/${bindir}/sedismod
	install test/dispol ${D}/${bindir}/sedispol
}

BBCLASSEXTEND = "native"
