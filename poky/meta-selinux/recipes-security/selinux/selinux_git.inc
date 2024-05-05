SRCREV = "1bac758bf6cf884c112b80545d5fc5b668fc7d71"

SRC_URI = "git://github.com/SELinuxProject/selinux.git;protocol=https"

include selinux_common.inc

# ${S} is set in selinux_common above, but we need to change it here since the
# top level directory is named differently
S = "${WORKDIR}/git/${BPN}"

DEFAULT_PREFERENCE = "-1"
