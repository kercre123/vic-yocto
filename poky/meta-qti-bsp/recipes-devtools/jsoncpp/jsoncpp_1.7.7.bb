inherit cmake pkgconfig

SUMMARY = "JSON C++ lib used to read and write json file."
DESCRIPTION = "Jsoncpp is an implementation of a JSON (http://json.org) reader \
               and writer in C++. JSON (JavaScript Object Notation) is a \
               lightweight data-interchange format. It is easy for humans to \
               read and write. It is easy for machines to parse and generate."

HOMEPAGE = "http://sourceforge.net/projects/jsoncpp/"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=c56ee55c03a55f8105b969d8270632ce"

# The revision of the recipe used to build the package.
PR = "r0"

FILESPATH =+ "${WORKSPACE}/external/:"
SRC_URI  := "file://jsoncpp"

S = "${WORKDIR}/jsoncpp"

EXTRA_OECMAKE += "-DJSONCPP_LIB_BUILD_SHARED=ON -DJSONCPP_WITH_TESTS=OFF"
