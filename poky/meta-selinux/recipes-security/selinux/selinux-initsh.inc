S ?= "${WORKDIR}"
SECTION ?= "base"

# Default is for script name to be the same as the recipe name.
# Script must have .sh suffix.
SELINUX_SCRIPT_SRC ?= "${BPN}"
SELINUX_SCRIPT_DST ?= "${SELINUX_SCRIPT_SRC}"

INITSCRIPT_NAME ?= "${SELINUX_SCRIPT_DST}"
INITSCRIPT_PARAMS ?= "start 00 S ."

CONFFILES_${PN} += "${sysconfdir}/init.d/${SELINUX_SCRIPT_DST}"

PACKAGE_ARCH ?= "${MACHINE_ARCH}"

inherit update-rc.d systemd

SYSTEMD_SERVICE_${PN} = "${SELINUX_SCRIPT_SRC}.service"

do_install () {
	install -d ${D}${sysconfdir}/init.d/
	install -m 0755 ${WORKDIR}/${SELINUX_SCRIPT_SRC}.sh ${D}${sysconfdir}/init.d/${SELINUX_SCRIPT_DST}

	install -d ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/${SELINUX_SCRIPT_SRC}.service ${D}${systemd_unitdir}/system

	if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
		install -d ${D}${bindir}
		install -m 0755 ${WORKDIR}/${SELINUX_SCRIPT_SRC}.sh ${D}${bindir}
	fi
}

sysroot_stage_all_append () {
	sysroot_stage_dir ${D}${sysconfdir} ${SYSROOT_DESTDIR}${sysconfdir}
}
