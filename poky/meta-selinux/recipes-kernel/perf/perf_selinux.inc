DEPENDS .= "${@bb.utils.contains('DISTRO_FEATURES', 'selinux', ' audit', '', d)}"
