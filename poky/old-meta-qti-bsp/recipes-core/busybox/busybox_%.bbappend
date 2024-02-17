FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "\
            file://find-touchscreen.sh \
            file://automountsdcard.sh \
            file://usb.sh \
            file://mdev.conf \
            file://profile \
            file://fstab \
            file://inittab \
            file://rcS \
            file://no-console.cfg \
            file://login.cfg \
            file://mdev.cfg \
            file://base.cfg \
            file://syslog-startup.conf \
            file://busybox-syslog.service \
            file://busybox_klogd.patch;patchdir=.. \
            file://iio.sh \
            file://0001-Support-MTP-function.patch \
            file://0001-Fix-file-synchronization-in-mdev.patch \
            file://fix-mdev-crash.patch \
"

prefix = ""

BUSYBOX_SPLIT_SUID = "0"

do_install_append() {
    # systemd is udev compatible.
    if ${@base_contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${sysconfdir}/udev/scripts/
        install -m 0755 ${WORKDIR}/automountsdcard.sh \
            ${D}${sysconfdir}/udev/scripts/automountsdcard.sh
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${WORKDIR}/busybox-syslog.service -D ${D}${systemd_unitdir}/system/busybox-syslog.service
        install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
    else
        install -d ${D}${sysconfdir}/mdev
        install -m 0755 ${WORKDIR}/automountsdcard.sh ${D}${sysconfdir}/mdev/
        install -m 0755 ${WORKDIR}/find-touchscreen.sh ${D}${sysconfdir}/mdev/
        install -m 0755 ${WORKDIR}/usb.sh ${D}${sysconfdir}/mdev/
        install -m 0755 ${WORKDIR}/iio.sh ${D}${sysconfdir}/mdev/
    fi
    mkdir -p ${D}/usr/bin
    ln -s /bin/env ${D}/usr/bin/env
}

python do_package_append_mdm() {
    import subprocess
    subprocess.call('rm -f ${D}/../packages-split/busybox/usr/lib/busybox/sbin/modprobe', shell=True)
}

#FILES_${PN}-mdev += "${sysconfdir}/mdev/* "
