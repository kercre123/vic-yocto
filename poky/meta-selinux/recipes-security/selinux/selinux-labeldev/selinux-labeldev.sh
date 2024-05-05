#!/bin/sh

/usr/sbin/selinuxenabled 2>/dev/null || exit 0

CHCON=/usr/bin/chcon
MATCHPATHCON=/usr/sbin/matchpathcon
RESTORECON=/sbin/restorecon

for i in ${CHCON} ${MATCHPATHCON} ${RESTORECON}; do
	test -x $i && continue
	echo "$i is missing in the system."
	echo "Please add \"selinux=0\" in the kernel command line to disable SELinux."
	exit 1
done

# Because /dev/console is not relabeled by kernel, many commands
# would can not use it, including restorecon.
${CHCON} -t `${MATCHPATHCON} -n /dev/null | cut -d: -f3` /dev/null
${CHCON} -t `${MATCHPATHCON} -n /dev/console | cut -d: -f3` /dev/console

# Now, we should relabel /dev for most services.
${RESTORECON} -RF /dev

exit 0
