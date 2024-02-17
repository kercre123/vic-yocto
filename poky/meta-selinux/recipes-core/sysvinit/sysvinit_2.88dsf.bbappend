require ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', 'sysvinit-2.88dsf_selinux.inc', '', d)}
