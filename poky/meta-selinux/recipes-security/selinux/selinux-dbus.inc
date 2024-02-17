SUMMARY = "SELinux dbus service files"
DESCRIPTION = "\
Provide SELinux dbus service files and scripts."

SECTION = "base"
LICENSE = "GPLv2+"

RDEPENDS_${PN} += "python selinux-python-sepolicy"

FILES_${PN} += "\
        ${datadir}/system-config-selinux/selinux_server.py \
        ${datadir}/polkit-1/actions/org.selinux.policy \
        ${datadir}/dbus-1/system-services/org.selinux.service \
"
