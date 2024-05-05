SUMMARY = "MLS (Multi Level Security) variant of the SELinux policy"
DESCRIPTION = "\
This is the reference policy for SE Linux built with MLS support. \
It allows giving data labels such as \"Top Secret\" and preventing \
such data from leaking to processes or files with lower classification. \
"

POLICY_TYPE = "mls"

include refpolicy_${PV}.inc
