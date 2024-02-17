inherit autotools-brokensep module qperf
DESCRIPTION = "Shortcut Forward Engine Driver"
LICENSE = "ISC"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=f3b90e78ea0cffb20bf5cca7947a896d"

PR = "${@oe.utils.conditional('PRODUCT', 'psm', 'r0-psm', 'r0', d)}"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://shortcut-fe/shortcut-fe/ "

S = "${WORKDIR}/shortcut-fe/shortcut-fe"
FILES_${PN}     += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/*"

do_install() {
    module_do_install
}

do_module_signing() {
    if [ ${BASEMACHINE} == qcs40x ]; then
    if [ -f  ${STAGING_KERNEL_BUILDDIR}/certs/signing_key.pem ]; then
	    bbnote "Signing ${PN} module ${i}"
        for i in $(find ${PKGDEST}/${PN}/${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/ -name "*.ko"); do
          ${STAGING_KERNEL_BUILDDIR}/scripts/sign-file sha512 ${STAGING_KERNEL_BUILDDIR}/certs/signing_key.pem ${STAGING_KERNEL_BUILDDIR}/certs/signing_key.x509 ${i}
        done
    else
        bbnote "${PN} module is not being signed"
    fi
    fi
}

addtask module_signing after do_package before do_package_write_ipk

RPROVIDES_${PN} += "${@'kernel-module-shortcut-fe-cm-${KERNEL_VERSION}'.replace('_', '-')}"
RPROVIDES_${PN} += "${@'kernel-module-shortcut-fe-ipv6-${KERNEL_VERSION}'.replace('_', '-')}"
RPROVIDES_${PN} += "${@'kernel-module-shortcut-fe-${KERNEL_VERSION}'.replace('_', '-')}"