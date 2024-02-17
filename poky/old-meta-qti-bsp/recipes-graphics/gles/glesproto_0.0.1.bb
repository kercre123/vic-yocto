DESCRIPTION = "GLES headers"
HOMEPAGE = "http://www.khronos.org/"
LICENSE = "FreeB-2"
LIC_FILES_CHKSUM = "file://include/GLES/gl.h;startline=12;endline=15;md5=4353e0b39204009be8445dc056b20d96"

DEPENDS = "libx11"
PR = "r4"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://base/opengl/include"
S = "${WORKDIR}/base/opengl/include"

ALLOW_EMPTY_${PN} = "1"

do_install() {
   for i in  EGL KHR GLES GLES2; do
      install -d ${D}/${includedir}/$i
      cp -pPr ${S}/$i/* ${D}/${includedir}/$i
   done
}

