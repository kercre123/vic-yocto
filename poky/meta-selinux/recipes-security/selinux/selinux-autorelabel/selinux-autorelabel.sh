#!/bin/sh

/usr/sbin/selinuxenabled 2>/dev/null || exit 0

FIXFILES=/sbin/fixfiles

if ! test -x ${FIXFILES}; then
	echo "${FIXFILES} is missing in the system."
	echo "Please add \"selinux=0\" in the kernel command line to disable SELinux."
	exit 1
fi

# If /.autorelabel placed, the whole file system should be relabeled
if [ -f /.autorelabel ]; then
	echo "SELinux: /.autorelabel placed, filesystem will be relabeled..."
	${FIXFILES} -F -f relabel
	/bin/rm -f /.autorelabel
	echo " * Relabel done, rebooting the system."
	/sbin/reboot
fi

exit 0
