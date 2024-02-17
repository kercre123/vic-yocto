DESCRIPTION = "QuIC Linux Kernel"
SECTION = "kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

# We don't want to pull in update-modules or depmod, like the base class does.
DEPENDS := "virtual/${TARGET_PREFIX}gcc \
            virtual/${TARGET_PREFIX}gcc${KERNEL_CCSUFFIX}"

inherit kernel

PACKAGES =+ "kernel-headers"
INSTALL_HDR_PATH="${STAGING_KERNEL_DIR}/usr"
FILES_kernel-headers += "${INSTALL_HDR_PATH}"

do_configure_prepend () {
    oe_runmake ${KERNEL_EXTRA_OEMAKE} ${KERNEL_DEFCONFIG}
}

do_install_append() {
    oe_runmake headers_install INSTALL_HDR_PATH=${INSTALL_HDR_PATH}
}

do_savedefconfig() {
    oe_runmake savedefconfig
}
addtask savedefconfig after do_configure
