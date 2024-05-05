DESCRIPTION = "SELinux packagegroup with only packages required for basic operations"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGES = "\
	${PN} \
"

ALLOW_EMPTY_${PN} = "1"

RDEPENDS_${PN} = "\
	coreutils \
	libsepol \
	libselinux \
	libselinux-bin \
	libsemanage \
	policycoreutils-fixfiles \
	policycoreutils-secon \
	policycoreutils-semodule \
	policycoreutils-sestatus \
	policycoreutils-setfiles \
	selinux-labeldev \
	refpolicy \
"
