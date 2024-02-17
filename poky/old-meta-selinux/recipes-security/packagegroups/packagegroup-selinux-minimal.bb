DESCRIPTION = "SELinux packagegroup with only packages required for basic operations"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
PR = "r0"

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
	selinux-config \
	selinux-labeldev \
	refpolicy-mls \
"
