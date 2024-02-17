inherit autotools

DESCRIPTION = "Genlock"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=3775480a712fc46a69647678acb234cb"

PR = "r3"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://graphics/libgenlock"
S = "${WORKDIR}/graphics/libgenlock"

DEPENDS += "virtual/kernel"

EXTRA_OECONF_append += " --with-kernel-headers=${STAGING_KERNEL_DIR}/usr/include"

PACKAGE_ARCH = "${MACHINE_ARCH}"

LEAD_SONAME="libgenlock.so"
FILES_${PN} += "/usr/lib/*.so"

CFLAGS += " -Wno-error "

do_install() {
   install -d ${D}/usr/include
   install -m 0644 ${S}/genlock.h ${D}/usr/include/

   install -d ${D}/usr/lib
      for lib in libgenlock.so; do
         install -m 0644 ${S}/.libs/${lib} -D ${D}/usr/lib/${lib}
   done
}
