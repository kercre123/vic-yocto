FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

#SRC_URI is pointing to CAF
SRC_URI = "${CLO_LE_GIT}/libsolv.git;protocol=https;branch=caf_migration/libsolv/master"
SRC_URI += "file://CVE-2019-20387.patch \
            file://CVE-2021-33928.patch \
"
