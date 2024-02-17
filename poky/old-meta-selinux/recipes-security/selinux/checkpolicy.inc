SUMMARY = "SELinux policy compiler"
DESCRIPTION = "\
This package contains checkpolicy, the SELinux policy compiler. Only \
required for building policies. It uses libsepol to generate the \
binary policy. checkpolicy uses the static libsepol since it deals \
with low level details of the policy that have not been \
encapsulated/abstracted by a proper shared library interface."

SECTION = "base"
LICENSE = "GPLv2+"

DEPENDS += "libsepol libselinux bison-native flex-native"

SRC_URI += "file://checkpolicy-Do-not-link-against-libfl.patch"

EXTRA_OEMAKE += "PREFIX=${D}" 
EXTRA_OEMAKE += "LEX='flex'"

do_install_append() {
	install test/dismod ${D}/${bindir}/sedismod
	install test/dispol ${D}/${bindir}/sedispol
}

BBCLASSEXTEND = "native"
