SRCREV = "edc2e99687b050d5be21a78a66d038aa1fc068d9"

SRC_URI = "git://github.com/SELinuxProject/selinux.git;protocol=http"

include selinux_common.inc

# ${S} is set in selinux_common above, but we need to change it here since the
# top level directory is named differently
S = "${WORKDIR}/git/${BPN}"

DEFAULT_PREFERENCE = "-1"
