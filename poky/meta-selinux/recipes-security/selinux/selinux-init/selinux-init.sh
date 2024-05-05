#!/bin/sh

/usr/sbin/selinuxenabled 2>/dev/null || exit 0

CHCON=/usr/bin/chcon
MATCHPATHCON=/usr/sbin/matchpathcon
RESTORECON=/sbin/restorecon
SECON=/usr/bin/secon
SETENFORCE=/usr/sbin/setenforce

for i in ${CHCON} ${MATCHPATHCON} ${RESTORECON} ${SECON} ${SETENFORCE}; do
	test -x $i && continue
	echo "$i is missing in the system."
	echo "Please add \"selinux=0\" in the kernel command line to disable SELinux."
	exit 1
done

check_rootfs()
{
	${CHCON} `${MATCHPATHCON} -n /` / >/dev/null 2>&1 && return 0
	echo ""
	echo "* SELinux requires the root '/' filesystem support extended"
	echo "  filesystem attributes (XATTRs).  It does not appear that this"
	echo "  filesystem has extended attribute support or it is not enabled."
	echo ""
	echo "  - To continue using SELinux you will need to enable extended"
	echo "    attribute support on the root device."
	echo ""
	echo "  - To disable SELinux, please add \"selinux=0\" in the kernel"
	echo "    command line."
	echo ""
	echo "* Halting the system now."
	/sbin/shutdown -f -h now
}

# If first booting, the security context type of init would be
# "kernel_t", and the whole file system should be relabeled.
if [ "`${SECON} -t --pid 1`" = "kernel_t" ]; then
	echo "Checking SELinux security contexts:"
	check_rootfs
	echo " * First booting, filesystem will be relabeled..."
	test -x /etc/init.d/auditd && /etc/init.d/auditd start
	${SETENFORCE} 0
	${RESTORECON} -RF /
	${RESTORECON} -F /
	echo " * Relabel done, rebooting the system."
	/sbin/reboot
fi

exit 0
