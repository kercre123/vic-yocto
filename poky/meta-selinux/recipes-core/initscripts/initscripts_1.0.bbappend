require ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'initscripts-1.0_selinux.inc', '', d)}
