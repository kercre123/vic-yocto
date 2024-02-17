DEFAULT_POLICY ??= "mls"
DEFAULT_ENFORCING ??= "enforcing"

SUMMARY = "SELinux configuration"
DESCRIPTION = "\
SELinux configuration files for Yocto. \
"

SECTION = "base"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
PR = "r4"

S = "${WORKDIR}"

CONFFILES_${PN} += "${sysconfdir}/selinux/config"

PACKAGE_ARCH = "${MACHINE_ARCH}"

do_install () {
	echo "\
# This file controls the state of SELinux on the system.
# SELINUX= can take one of these three values:
#     enforcing - SELinux security policy is enforced.
#     permissive - SELinux prints warnings instead of enforcing.
#     disabled - No SELinux policy is loaded.
SELINUX=${DEFAULT_ENFORCING}
# SELINUXTYPE= can take one of these values:
#     standard - Standard Security protection.
#     mls - Multi Level Security protection.
#     targeted - Targeted processes are protected.
#     mcs - Multi Category Security protection.
SELINUXTYPE=${DEFAULT_POLICY}
" > ${WORKDIR}/config
	install -d ${D}/${sysconfdir}/selinux
	install -m 0644 ${WORKDIR}/config ${D}/${sysconfdir}/selinux/
}

sysroot_stage_all_append () {
	sysroot_stage_dir ${D}${sysconfdir} ${SYSROOT_DESTDIR}${sysconfdir}
}
