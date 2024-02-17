FILESEXTRAPATHS_append := "${THISDIR}/files:"

DEPENDS_${BPN}-setfiles += "openssl"
RDEPENDS_${BPN}-setfiles += "openssl"

SRC_URI += "file://add_hash_and_lost_and_found.patch"
