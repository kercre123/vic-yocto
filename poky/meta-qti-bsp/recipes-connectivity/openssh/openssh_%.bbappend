#
# <anki>
# VIC-2065: PasswordAuthentication=no
# </anki>
#
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://sshdgenkeys.service \
      file://sshd_config \
      file://ssh_host_rsa_key \
      file://ssh_host_dsa_key \
      file://ssh_host_ecdsa_key \
      file://ssh_host_ed25519_key \
      file://ssh_root_key.pub \
      "

# EXTRA_OECONF += " --sysconfdir=/data/ssh"

EXTRA_OECONF_append=" ${@bb.utils.contains('DISTRO_FEATURES', 'selinux', '--with-selinux', '', d)}"
BASEPRODUCT = "${@d.getVar('PRODUCT', False)}"
do_install_append () {
    if [ "${BASEPRODUCT}" == "drone" ]; then
        sed -i -e 's:#PermitRootLogin yes:PermitRootLogin yes:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config
        sed -i -e 's:#PasswordAuthentication yes:PasswordAuthentication yes:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config
    elif [ "${BASEPRODUCT}" == "robot" ] || [ "${BASEPRODUCT}" == "robot-rome" ]; then
        sed -i -e 's:#PermitRootLogin yes:PermitRootLogin yes:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config
        sed -i -e 's:#PasswordAuthentication yes:PasswordAuthentication no:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config
        sed -i -e 's:#PermitRootLogin yes:PermitRootLogin yes:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config_readonly
        sed -i -e 's:#PasswordAuthentication yes:PasswordAuthentication no:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config_readonly
        sed -i '$a    StrictHostKeyChecking no' ${WORKDIR}/ssh_config ${D}${sysconfdir}/ssh/ssh_config
        sed -i '$a    UserKnownHostsFile /dev/null' ${WORKDIR}/ssh_config ${D}${sysconfdir}/ssh/ssh_config
        sed -i -e 's:AuthorizedKeysFile .ssh/authorized_keys:AuthorizedKeysFile .ssh/authorized_keys /etc/ssh/authorized_keys:' ${WORKDIR}/sshd_config ${D}${sysconfdir}/ssh/sshd_config
    fi
    install -m 0600 ${WORKDIR}/ssh_host_rsa_key ${D}${sysconfdir}/ssh/ssh_host_rsa_key
    install -m 0600 ${WORKDIR}/ssh_host_dsa_key ${D}${sysconfdir}/ssh/ssh_host_dsa_key
    install -m 0600 ${WORKDIR}/ssh_host_ecdsa_key ${D}${sysconfdir}/ssh/ssh_host_ecdsa_key
    install -m 0600 ${WORKDIR}/ssh_host_ed25519_key ${D}${sysconfdir}/ssh/ssh_host_ed25519_key
    install -m 0600 ${WORKDIR}/ssh_root_key.pub ${D}${sysconfdir}/ssh/authorized_keys
}

RDEPENDS_${PN} += "${PN}-sftp"
