# overwirte EXTRA_OECONF
EXTRA_OECONF := "${@d.getVar('EXTRA_OECONF', True).replace('--without-selinux', '')}"
EXTRA_OECONF += "${@bb.utils.contains('DISTRO_FEATURES','selinux',' --with-selinux',' --without-selinux',d)}"
