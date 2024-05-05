SUMMARY = "Protocol Buffers - structured data serialisation mechanism"
DESCRIPTION = "Protocol Buffers are a way of encoding structured data in an \
efficient yet extensible format. Google uses Protocol Buffers for almost \
all of its internal RPC protocols and file formats."
HOMEPAGE = "https://github.com/google/protobuf"
SECTION = "console/tools"
LICENSE = "BSD-3-Clause"

SRCNAME = "protobuf"

PACKAGE_BEFORE_PN = "${PN}-compiler"

DEPENDS = "zlib python-setuptools-native python-native protobuf-native"

LIC_FILES_CHKSUM = "file://setup.py;begineline=237;endline=237;md5=280e00a114b06867a5b7ec32779b6c61"

SRCREV = "a6189acd18b00611c1dc7042299ad75486f08a1a"

#PV = "3.3.0+git${SRCPV}"

SRC_URI = "${CLO_LE_GIT}/protobuf.git;protocol=https;branch=caf_migration/protobuf/master"

S = "${WORKDIR}/git/python"

BBCLASSEXTEND = "native nativesdk"

inherit distutils

# The installer doesn't seem to add path to protobuf. Correct it.
# Cherry-pick the files after the `setup.py install` and copy them to ${D}.
do_install() {
    install -d ${D}${PYTHON_SITEPACKAGES_DIR}

    # this run installs the egg file in to python2.7/site-packages folder
    STAGING_INCDIR=${STAGING_INCDIR} \
    STAGING_LIBDIR=${STAGING_LIBDIR} \
    PYTHONPATH=${D}${PYTHON_SITEPACKAGES_DIR} \
    BUILD_SYS=${BUILD_SYS} HOST_SYS=${HOST_SYS} \
    ${STAGING_BINDIR_NATIVE}/${PYTHON_PN}-native/${PYTHON_PN} setup.py install --install-lib=${D}/${PYTHON_SITEPACKAGES_DIR} || \
        bbfatal "${PYTHON_PN} setup.py install execution failed."

    # the above fails to add the path, just install that into ${D}
    rm -f ${D}${PYTHON_SITEPACKAGES_DIR}/protobuf.pth
    echo "./${SRCNAME}-${PV}-py${PYTHON_BASEVERSION}.egg" > ${D}${PYTHON_SITEPACKAGES_DIR}/protobuf.pth
#    cp "${S}/dist/${SRCNAME}-${PV}-py${PYTHON_BASEVERSION}.egg" ${D}${PYTHON_SITEPACKAGES_DIR}
#    cp "${S}/${SRCNAME}.egg-info/PKG-INFO" "${D}${PYTHON_SITEPACKAGES_DIR}/${SRCNAME}-${PV}-py${PYTHON_BASEVERSION}.egg-info"
}


