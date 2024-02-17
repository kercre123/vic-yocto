SUMMARY = "SELinux Common Intermediate Language (CIL) compiler"
DESCRIPTION = "\
This package contains secilc, the SELinux Common Intermediate \
Language (CIL) compiler."

SECTION = "base"
LICENSE = "BSD"

DEPENDS += "libsepol xmlto-native"

BBCLASSEXTEND = "native"
