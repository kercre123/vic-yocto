DEPENDS += "mesa-dri-glsl-native"

#file://uclibc.patch 
#file://mesa_fix_for_x32.patch 

SRC_URI = "ftp://ftp.freedesktop.org/pub/mesa/${PV}/MesaLib-${PV}.tar.bz2 \ 
           file://crossfix.patch \
           file://crossfix-mklib.patch \
           "
S = "${WORKDIR}/Mesa-${PV}"

SRC_URI[md5sum] = "d546f988adfdf986cff45b1efa2d8a46"
SRC_URI[sha256sum] = "02ed19f4f5f6535dda03a9932a81438aa78ea723ebba1f39a3d49a70a4e1d07e"

#check for python not python2, because python-native does not stage python2 binary/link
# We need builtin_compiler built for buildhost arch instead of target (is provided by mesa-dri-glsl-native)"
do_configure_prepend() {
  sed -i 's/AC_CHECK_PROGS(\[PYTHON2\], \[python2 python\])/AC_CHECK_PROGS(\[PYTHON2\], \[python python\])/g' ${S}/configure.ac
  sed -i "s#\./builtin_compiler#${STAGING_BINDIR_NATIVE}/glsl/builtin_compiler#g" ${S}/src/glsl/Makefile
}
