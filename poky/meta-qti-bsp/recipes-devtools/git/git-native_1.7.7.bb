require recipes-devtools/git/git.inc
inherit native
SRC_URI[md5sum] = "5d645884e688921e773186783b65ce33"
SRC_URI[sha256sum] = "5a977bc01e4989b9928345e99aab15ce896cf5897c6e32eb449538574df377f6"
SRC_URI = "http://git-core.googlecode.com/files/git-1.7.7.tar.gz"
DEPENDS = "perl-native openssl-native curl-native zlib-native expat-native"
PR = "r0"

EXTRA_OECONF_append = " --without-python"
EXTRA_OEMAKE = "NO_TCLTK=1"
