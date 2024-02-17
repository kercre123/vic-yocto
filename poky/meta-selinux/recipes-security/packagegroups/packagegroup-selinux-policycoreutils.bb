DESCRIPTION = "SELinux policycoreutils packagegroup"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGES = "\
    ${PN} \
"

ALLOW_EMPTY_${PN} = "1"

RDEPENDS_${PN} = "\
	policycoreutils-fixfiles \
	policycoreutils-genhomedircon \
	policycoreutils-loadpolicy \
	policycoreutils-newrole \
	policycoreutils-runinit \
	policycoreutils-secon \
	policycoreutils-semodule \
	policycoreutils-sestatus \
	policycoreutils-setfiles \
	policycoreutils-setsebool \
	policycoreutils-hll \
	semodule-utils-semodule-expand \
	semodule-utils-semodule-link \
	semodule-utils-semodule-package \
	selinux-python-audit2allow \
	selinux-python-sepolgen-ifgen \
	selinux-python-semanage \
	selinux-python-sepolicy \
	selinux-python-chcat \
	selinux-python \
	selinux-sandbox \
	selinux-dbus \
	selinux-gui \
"
