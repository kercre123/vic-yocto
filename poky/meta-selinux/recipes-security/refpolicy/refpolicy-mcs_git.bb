SUMMARY = "MCS (Multi Category Security) variant of the SELinux policy"
DESCRIPTION = "\
This is the reference policy for SE Linux built with MCS support. \
An MCS policy is the same as an MLS policy but with only one sensitivity \
level. This is useful on systems where a hierarchical policy (MLS) isn't \
needed (pretty much all systems) but the non-hierarchical categories are. \
"

POLICY_TYPE = "mcs"

include refpolicy_${PV}.inc
