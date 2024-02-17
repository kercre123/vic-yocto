inherit autotools pkgconfig

DESCRIPTION = "Build crypthelper-mediameta, a helper library\
               to provide mapping between encryption meta and\
               encryptable block devices"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r0"

SRC_URI   = "file://crypthelper-mediameta"
SRC_URI  += "file://media-encryption.conf"

S = "${WORKDIR}/crypthelper-mediameta"

CFLAGS += "-I${S}/libs"

do_install_append() {
    install -m 0644 ${WORKDIR}/media-encryption.conf -D ${D}/${sysconfdir}/conf/media-encryption.conf
}

do_install_append_sdm845 () {
    sed -i "s/footer/fdemeta/g" ${D}/${sysconfdir}/conf/media-encryption.conf
}

PACKAGES =+ "${PN}-lib"
FILES_${PN}-lib   =  "${sysconfdir}/conf/*"
FILES_${PN}-lib  +=  "${libdir}/libcrypthelper_mediameta.so.*  ${libdir}/pkgconfig/*"
