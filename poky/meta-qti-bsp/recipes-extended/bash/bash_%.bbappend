# Busybox installs sh with priority 50.
# Set bash priority less than that to get ignored in alternatives.
ALTERNATIVE_PRIORITY = "10"

pkg_prerm_${PN} () {
  # Remove /bin/bash from shell alternatives
  update-alternatives --remove sh /bin/bash
}
