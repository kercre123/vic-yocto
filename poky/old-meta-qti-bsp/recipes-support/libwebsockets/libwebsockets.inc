DESCRIPTION = "A lightweight pure C library built to use minimal CPU and memory resources, and provide fast throughput in both directions as client or server"
HOMEPAGE = "https://libwebsockets.org/"
SECTION = "networking"
MAINTAINER = "Andy Green"

LICENSE = "LGPL2.1 & BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e3393a657830d4a118c5a6ed35ba53d0"

SRC_URI = "https://github.com/warmcat/${PN}/archive/v${PV}.tar.gz;downloadfilename=${PN}-${PV}.tar.gz"

inherit cmake

FILES_${PN}-dev += "${libdir}/cmake"

EXTRA_OECMAKE = "-DCMAKE_INSTALL_PREFIX=/usr \
		${@base_conditional("libdir", "/usr/lib64", "-DLIB_SUFFIX=64", "", d)} \
		${@base_conditional("libdir", "/usr/lib32", "-DLIB_SUFFIX=32", "", d)} \
		-DLWS_WITH_STATIC=OFF \
		-DLWS_WITHOUT_TESTAPPS=ON \
		-DLWS_WITHOUT_TEST_SERVER=ON \
		-DLWS_UNIX_SOCK=ON"

PACKAGECONFIG ??= "openssl zlib libuv"

PACKAGECONFIG[libuv] = "-DLWS_WITH_LIBUV=ON -DLIBUV_LIBRARIES=${STAGING_DIR_HOST}/${libdir}/libuv.so,\
			-DLWS_WITH_LIBUV=OFF,libuv,"
PACKAGECONFIG[zlib] = "-DLWS_WITH_ZLIB=ON -DLWS_WITHOUT_EXTENSIONS=OFF,\
			-DLWS_WITH_SSL=OFF -DLWS_WITHOUT_EXTENSIONS=ON,zlib,"
PACKAGECONFIG[openssl] = "-DLWS_WITH_SSL=ON,-DLWS_WITH_SSL=OFF,openssl,"
