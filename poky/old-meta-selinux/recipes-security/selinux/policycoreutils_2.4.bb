include selinux_20150202.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=393a5ca445f6965873eca0259a17f833"

SRC_URI[md5sum] = "795b05c3ad58253cba61249ec65b28ef"
SRC_URI[sha256sum] = "b819f876f12473783ccce9f63b9a79cd77177477cd6d46818441f808cc4c3479"

SRC_URI += "\
	file://policycoreutils-fix-sepolicy-install-path.patch \
	file://policycoreutils-make-O_CLOEXEC-optional.patch \
	file://policycoreutils-loadpolicy-symlink.patch \
	file://policycoreutils-semanage-edit-user.patch \
	file://policycoreutils-process-ValueError-for-sepolicy-seobject.patch \
	file://policycoreutils-fix-TypeError-for-seobject.py.patch \
	file://0001-mcstrans-fix-the-init-script.patch \
	file://enable-mcstrans.patch \
	file://policycoreutils-fts_flags-FTS_NOCHDIR.patch \
	file://policycoreutils-pp-builtin-roles.patch \
	"
