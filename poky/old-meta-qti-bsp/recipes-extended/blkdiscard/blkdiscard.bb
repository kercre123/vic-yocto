DESCRIPTION = "blkdiscard utility forked from https://git.kernel.org/pub/scm/utils/util-linux/util-linux.git/tree/sys-utils/blkdiscard.c"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-qti-bsp/recipes-extended/blkdiscard/files/blkdiscard.c;beginline=7;endline=18;md5=909275dfe35bdbdd8d9ddc0484cf03a9"

SRC_URI = "file://blkdiscard.c"

do_compile () {
  ${CC} ${WORKDIR}/blkdiscard.c -o ${WORKDIR}/blkdiscard
}

do_install() {
  install -d ${D}/bin
  install -m 0755 ${WORKDIR}/blkdiscard ${D}/bin/
}

FILES_${PN} += "/bin/blkdiscard"
