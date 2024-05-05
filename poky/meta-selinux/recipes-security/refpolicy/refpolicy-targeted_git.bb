SUMMARY = "SELinux targeted policy"
DESCRIPTION = "\
This is the targeted variant of the SELinux reference policy.  Most service \
domains are locked down. Users and admins will login in with unconfined_t \
domain, so they have the same access to the system as if SELinux was not \
enabled. \
"

FILESEXTRAPATHS_prepend := "${THISDIR}/refpolicy-${PV}:"

POLICY_NAME = "targeted"
POLICY_TYPE = "mcs"
POLICY_MLS_SENS = "0"

include refpolicy_${PV}.inc

SRC_URI += " \
            file://refpolicy-fix-optional-issue-on-sysadm-module.patch \
            file://refpolicy-unconfined_u-default-user.patch \
            ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'file://refpolicy-remove-duplicate-type_transition.patch', '', d)} \
           "
