inherit ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'with-audit', '', d)}
inherit ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'with-selinux', '', d)}
