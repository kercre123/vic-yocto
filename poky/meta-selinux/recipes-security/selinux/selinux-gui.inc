SUMMARY = "SELinux GUI tools"
DESCRIPTION = "\
Provide SELinux Management tool (system-config-selinux) and SELinux \
Policy Generation Tool (selinux-polgengui)"

SECTION = "base"
LICENSE = "GPLv2+"

RDEPENDS_${PN} += "python"

FILES_${PN} += " \
        ${datadir}/system-config-selinux/* \
        ${datadir}/icons/hicolor/* \
        ${datadir}/polkit-1/actions/org.selinux.config.policy \
"
