DESCRIPTION = "Anki Robot Electronic Medical Record Reading Utility"
LICENSE = "Anki-Inc.-Proprietary"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-qti-bsp/files/anki-licenses/\
Anki-Inc.-Proprietary;md5=4b03b8ffef1b70b13d869dbce43e8f09"


SRC_URI = "file://emr-cat.c"

TARGET_CFLAGS += "-Os -Wall -Werror -Wno-unused-result -Wno-strict-aliasing -fPIC"

do_compile () {
  ${CC} ${TARGET_CFLAGS} ${WORKDIR}/emr-cat.c -o ${WORKDIR}/emr-cat
}

do_install() {
  install -d ${D}/bin
  install -m 0755 ${WORKDIR}/emr-cat ${D}/bin/
}

FILES_${PN} += "/bin/emr-cat"
