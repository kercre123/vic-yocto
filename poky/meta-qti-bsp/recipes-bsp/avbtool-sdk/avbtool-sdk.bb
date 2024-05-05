LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=0835ade698e0bcf8506ecda2f7b4f302"

DESCRIPTION = "avbtool: Image signing tool"
PR = "r1"

FILESEXTRAPATHS_prepend := "${THISDIR}/sigkeys:"
FILESPATH =+ "${WORKSPACE}/external/avb/:"

SRC_URI = "file://avbtool"
SRC_URI += "file://qpsa_attest.der"
SRC_URI += "file://qpsa_attest.key"

do_install() {
       install -d ${D}${datadir}/avb_py_tool
       install -d ${D}${datadir}/avb_py_tool/keys
       install -m 0555 ${WORKDIR}/avbtool ${D}${datadir}/avb_py_tool/
       install -m 0444 ${WORKDIR}/qpsa_attest.key ${D}${datadir}/avb_py_tool/keys/
       install -m 0444 ${WORKDIR}/qpsa_attest.der ${D}${datadir}/avb_py_tool/keys/
}

#don't run these functions
do_configure[noexec] = "1"
do_compile[noexec] = "1"

FILES_${PN} += " ${datadir}/avb_py_tool/*"
BBCLASSEXTEND = "nativesdk"
