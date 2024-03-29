SUMMARY = "Rotates, compresses, removes and mails system log files"
SECTION = "console/utils"
HOMEPAGE = "https://github.com/logrotate/logrotate/issues"
LICENSE = "GPLv2"

# TODO: Document coreutils dependency. Why not RDEPENDS? Why not busybox?

DEPENDS="coreutils popt"

LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

# When updating logrotate to latest upstream, SRC_URI should point to
# a proper release tarball from https://github.com/logrotate/logrotate/releases
# and we have to take the snapshot for now because there is no such
# tarball available for 3.9.1.

S = "${WORKDIR}/${BPN}-${PV}"

UPSTREAM_CHECK_URI = "https://github.com/${BPN}/${BPN}/releases"
UPSTREAM_CHECK_REGEX = "logrotate-(?P<pver>\d+(\.\d+)+).tar"

SRC_URI = "https://github.com/${BPN}/${BPN}/releases/download/${PV}/${BP}.tar.xz \
            file://act-as-mv-when-rotate.patch \
            file://update-the-manual.patch \
            file://disable-check-different-filesystems.patch \
            "

SRC_URI[md5sum] = "78ef24d6fddcc4df8e412dd75c551b4c"
SRC_URI[sha256sum] = "3222ca032f99be8d7a4a8c6ad69f3dcc49b9511bfe384bd5a271ebcd9bd3e52c"


PACKAGECONFIG[acl] = ",,acl"
PACKAGECONFIG[selinux] = ",,libselinux"

CONFFILES_${PN} += "${localstatedir}/lib/logrotate.status \
		    ${sysconfdir}/logrotate.conf"

# If RPM_OPT_FLAGS is unset, it adds -g itself rather than obeying our
# optimization variables, so use it rather than EXTRA_CFLAGS.
EXTRA_OEMAKE = "\
    LFS= \
    OS_NAME='${OS_NAME}' \
    'CC=${CC}' \
    'RPM_OPT_FLAGS=${CFLAGS}' \
    'EXTRA_LDFLAGS=${LDFLAGS}' \
    ${@bb.utils.contains('PACKAGECONFIG', 'acl', 'WITH_ACL=yes', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'selinux', 'WITH_SELINUX=yes', '', d)} \
"

# OS_NAME in the makefile defaults to `uname -s`. The behavior for
# freebsd/netbsd is questionable, so leave it as Linux, which only sets
# INSTALL=install and BASEDIR=/usr.
OS_NAME = "Linux"

inherit autotools systemd

SYSTEMD_SERVICE_${PN} = "\
    ${BPN}.service \
    ${BPN}.timer \
"

LOGROTATE_SYSTEMD_TIMER_BASIS ?= "daily"
LOGROTATE_SYSTEMD_TIMER_ACCURACY ?= "12h"

do_install(){
    oe_runmake install DESTDIR=${D} PREFIX=${D} MANDIR=${mandir}
    mkdir -p ${D}${sysconfdir}/logrotate.d
    mkdir -p ${D}${localstatedir}/lib
    install -p -m 644 ${S}/examples/logrotate-default ${D}${sysconfdir}/logrotate.conf
    touch ${D}${localstatedir}/lib/logrotate.status

    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/examples/logrotate.service ${D}${systemd_system_unitdir}/logrotate.service
        install -m 0644 ${S}/examples/logrotate.timer ${D}${systemd_system_unitdir}/logrotate.timer
        sed -i -e 's,OnCalendar=.*$,OnCalendar=${LOGROTATE_SYSTEMD_TIMER_BASIS},g' ${D}${systemd_system_unitdir}/logrotate.timer
        sed -i -e 's,AccuracySec=.*$,AccuracySec=${LOGROTATE_SYSTEMD_TIMER_ACCURACY},g' ${D}${systemd_system_unitdir}/logrotate.timer
    fi

    if ${@bb.utils.contains('DISTRO_FEATURES', 'sysvinit', 'true', 'false', d)}; then
        mkdir -p ${D}${sysconfdir}/cron.daily
        install -p -m 0755 ${S}/examples/logrotate.cron ${D}${sysconfdir}/cron.daily/logrotate
    fi
}
