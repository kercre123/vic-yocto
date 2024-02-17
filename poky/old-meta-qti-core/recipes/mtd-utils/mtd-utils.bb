DESCRIPTION = "Tools for managing memory technology devices."
SECTION = "base"
DEPENDS = "zlib lzo e2fsprogs util-linux"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=0636e73ff0215e8d672dc4c32c317bb3 \
                    file://include/common.h;beginline=1;endline=17;md5=ba05b07912a44ea2bf81ce409380049c"

FILESPATH =+ "${WORKSPACE}/filesystems:"
SRC_URI = "file://mtd-utils"

S = "${WORKDIR}/mtd-utils"

PR = "r1"

EXTRA_OEMAKE = "'CC=${CC}' 'RANLIB=${RANLIB}' 'AR=${AR}' 'CFLAGS=${CFLAGS} -I${S}/include -I${S}/ubi-utils/include -I${S}/tests/fs-tests/lib -DWITHOUT_XATTR' 'BUILDDIR=${S}'"

do_compile_append () {
	oe_runmake tests
}

ubi_tests = " \
	integ \
	io_basic \
	io_paral \
	io_read \
	io_update \
	mkvol_bad \
	mkvol_basic \
	mkvol_paral \
	rsvol \
	volrefcnt \
	"

checkfs_tests = " \
	checkfs \
	makefiles \
	"

MTD_TEST_BIN_PATH = "${WORKSPACE}/filesystems/bin/target/mtd-utils"
do_install () {
	oe_runmake install DESTDIR=${D} SBINDIR=${sbindir} MANDIR=${mandir} INCLUDEDIR=${includedir}

	mkdir -p ${MTD_TEST_BIN_PATH}/fstests/
	find ${S}/tests/fs-tests/ -executable -type f -exec cp {} ${MTD_TEST_BIN_PATH}/fstests/ \;

	mkdir -p ${MTD_TEST_BIN_PATH}/ubi-tests/
	for test in ${ubi_tests}; do
		cp ${S}/$test ${MTD_TEST_BIN_PATH}/ubi-tests/
	done

	mkdir -p ${MTD_TEST_BIN_PATH}/checkfs/
	for test in ${checkfs_tests}; do
		cp ${S}/$test ${MTD_TEST_BIN_PATH}/checkfs/
	done
}


PACKAGES =+ "mtd-utils-jffs2 mtd-utils-ubifs mtd-utils-misc"

FILES_mtd-utils-jffs2 = "${sbindir}/mkfs.jffs2 ${sbindir}/jffs2dump ${sbindir}/jffs2reader ${sbindir}/sumtool"
FILES_mtd-utils-ubifs = "${sbindir}/mkfs.ubifs ${sbindir}/ubi*"
FILES_mtd-utils-misc = "${sbindir}/nftl* ${sbindir}/ftl* ${sbindir}/rfd* ${sbindir}/doc* ${sbindir}/serve_image ${sbindir}/recv_image"

PARALLEL_MAKE = ""

BBCLASSEXTEND = "native"
