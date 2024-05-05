DESCRIPTION = "SELinux packagegroup for Poky"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGES = "\
    ${PN} \
    "

ALLOW_EMPTY_${PN} = "1"

RDEPENDS_${PN} = " \
	libsepol \
	libsepol-bin \
	libselinux \
	libselinux-bin \
	libsemanage \
	checkpolicy \
	selinux-python-sepolgen \
	packagegroup-selinux-policycoreutils \
	setools \
	setools-console \
	selinux-autorelabel \
	selinux-init \
	selinux-labeldev \
	refpolicy \
	coreutils \
	"
