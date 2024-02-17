inherit with-selinux

PACKAGECONFIG_append = "${@bb.utils.contains('DISTRO_FEATURES', 'acl', ' acl', '', d)}"
