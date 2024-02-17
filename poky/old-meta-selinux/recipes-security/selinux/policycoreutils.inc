SUMMARY = "SELinux policy core utilities"
DESCRIPTION = "policycoreutils contains the policy core utilities that are required \
for basic operation of a SELinux system.  These utilities include \
load_policy to load policies, setfiles to label filesystems, newrole \
to switch roles, and run_init to run /etc/init.d scripts in the proper \
context."
SECTION = "base"
LICENSE = "GPLv2+"

SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', 'pam', '${PAM_SRC_URI}', '', d)} \
            file://policycoreutils-fixfiles-de-bashify.patch \
            file://policycoreutils-sandbox-de-bashify.patch \
            file://mcstrans-de-bashify.patch \
           "

PAM_SRC_URI = "file://pam.d/newrole \
               file://pam.d/run_init \
"

DEPENDS += "libsepol libselinux libsemanage libcap"
EXTRA_DEPENDS = "libcap-ng libcgroup setools"
DEPENDS += "${@['', '${EXTRA_DEPENDS}']['${PN}' != '${BPN}-native']}"

inherit selinux systemd pythonnative update-rc.d

PROVIDES += "mcstrans"

DEPENDS += "${@target_selinux(d, 'libpam audit')}"

RDEPENDS_${BPN}-audit2allow = "\
	python-textutils \
	libselinux-python \
	sepolgen \
"
RDEPENDS_${BPN}-chcat = "\
	python-codecs \
	python-shell \
	python-stringold \
	python-unixadmin \
	${BPN}-python \
	libselinux-python \
"
RDEPENDS_${BPN}-fixfiles += "\
	${BPN}-setfiles \
"
RDEPENDS_${BPN}-genhomedircon += "\
	${BPN}-genhomedircon \
	${BPN}-semodule \
"
RDEPENDS_${BPN}-loadpolicy += "\
	libselinux \
	libsepol \
"
RDEPENDS_${BPN}-newrole += "\
	libcap-ng \
	libselinux \
"
RDEPENDS_${BPN}-python += "\
	python-codecs \
	python-io \
	python-ipy \
	python-re \
	python-stringold \
	python-syslog \
	python-unixadmin \
	libselinux-python \
	libsemanage-python \
"
RDEPENDS_${BPN}-runinit += "libselinux"
RDEPENDS_${BPN}-sandbox += "\
	python-math \
	python-shell \
	python-subprocess \
	python-textutils \
	python-unixadmin \
	libselinux-python \
	${BPN}-python \
"
RDEPENDS_${BPN}-secon += "libselinux"
RDEPENDS_${BPN}-semanage = "\
	python-core \
	python-ipy \
	python-compression \
	python-xml \
	${BPN}-python \
	libselinux-python \
"
RDEPENDS_${BPN}-semodule += "\
	libsepol \
	libselinux \
	libsemanage \
"
# static link to libsepol
DEPENDS_${BPN}-semodule-deps += "libsepol"
RDEPENDS_${BPN}-semodule-expand += "libsepol libselinux"
RDEPENDS_${BPN}-semodule-link += "libsepol libselinux"
RDEPENDS_${BPN}-semodule-package += "libsepol libselinux"
RDEPENDS_${BPN}-sepolicy += "\
	python-argparse \
	python-codecs \
	python-core \
	python-syslog \
	${BPN}-python \
"
# static link to libsepol
DEPENDS_${BPN}-sepolgen-ifgen += "libsepol"
RDEPENDS_${BPN}-sepolgen-ifgen += "python libselinux-python"
RDEPENDS_${BPN}-sestatus += "libselinux"
RDEPENDS_${BPN}-setfiles += "\
	libselinux \
	libsepol \
"
RDEPENDS_${BPN}-setsebool += "\
	libsepol \
	libselinux \
	libsemanage \
"
RDEPENDS_${BPN} += "setools setools-libs ${BPN}-python"

WARN_QA := "${@oe_filter_out('unsafe-references-in-scripts', '${WARN_QA}', d)}"
ERROR_QA := "${@oe_filter_out('unsafe-references-in-scripts', '${ERROR_QA}', d)}"


PACKAGES =+ "\
	${PN}-audit2allow \
	${PN}-chcat \
	${PN}-fixfiles \
	${PN}-genhomedircon \
	${PN}-hll \
	${PN}-loadpolicy \
	${PN}-newrole \
	${PN}-python \
	${PN}-runinit \
	${PN}-sandbox \
	${PN}-secon \
	${PN}-semanage \
	${PN}-semodule \
	${PN}-semodule-deps \
	${PN}-semodule-expand \
	${PN}-semodule-link \
	${PN}-semodule-package \
	${PN}-sepolgen-ifgen \
	${PN}-sepolicy \
	${PN}-sestatus \
	${PN}-setfiles \
	${PN}-setsebool \
	mcstrans \
	mcstrans-doc \
	system-config-selinux \
"
PKGV_mcstrans = "0.3.2"
PKGV_mcstrans-doc = "0.3.2"
SUMMARY_mcstrans = "Daemon to translate SELinux MCS/MLS sensitivity labels"
DESCRIPTION_mcstrans = "\
        Security-enhanced Linux is a feature of the Linux kernel and a number \
        of utilities with enhanced security functionality designed to add \
        mandatory access controls to Linux.  The Security-enhanced Linux \
        kernel contains new architectural components originally developed to \
        improve the security of the Flask operating system. These \
        architectural components provide general support for the enforcement \
        of many kinds of mandatory access control policies, including those \
        based on the concepts of Type Enforcement®, Role-based Access \
        Control, and Multi-level Security. \
         \
        mcstrans provides an translation daemon to translate SELinux categories \
        from internal representations to user defined representation. \
        "
SUMMARY_mcstrans-doc = "${SUMMARY_mcstrans} man pages and examples"
DESCRIPTION_mcstrans-doc = "${DESCRIPTION_mcstrans} \
        This package contains man pages and examples. \
        "
FILES_${PN}-audit2allow = "\
	${bindir}/audit2allow \
	${bindir}/audit2why \
"
FILES_${PN}-chcat = "\
	${bindir}/chcat \
"
FILES_${PN}-fixfiles += "${base_sbindir}/fixfiles"
FILES_${PN}-genhomedircon += "${sbindir}/genhomedircon"
FILES_${PN}-loadpolicy += "\
	${base_sbindir}/load_policy \
	${sbindir}/load_policy \
"
FILES_${PN}-newrole += "\
	${bindir}/newrole \
	${@bb.utils.contains('DISTRO_FEATURES', 'pam', '${sysconfdir}/pam.d/newrole', '', d)} \
"
FILES_${PN}-python = "\
	${libdir}/python${PYTHON_BASEVERSION}/site-packages/seobject.py* \
	${libdir}/python${PYTHON_BASEVERSION}/site-packages/sepolicy*.egg-info \
	${libdir}/python${PYTHON_BASEVERSION}/site-packages/sepolicy/* \
"
FILES_${PN}-runinit += "\
	${sbindir}/run_init \
	${sbindir}/open_init_pty \
	${@bb.utils.contains('DISTRO_FEATURES', 'pam', '${sysconfdir}/pam.d/run_init', '', d)} \
"
FILES_${PN}-dbg += "${libdir}/python${PYTHON_BASEVERSION}/site-packages/sepolicy/.debug/* ${prefix}/libexec/selinux/hll/.debug"
FILES_${PN}-sandbox += "\
	${datadir}/sandbox/* \
	${bindir}/sandbox \
	${sbindir}/seunshare \
	${sysconfdir}/sysconfig/sandbox \
"
FILES_${PN}-secon += "${bindir}/secon"
FILES_${PN}-semanage = "\
	${sbindir}/semanage \
	${datadir}/bash-completion/completions/semanage \
"
FILES_${PN}-semodule += "${sbindir}/semodule"
FILES_${PN}-semodule-deps += "${bindir}/semodule_deps"
FILES_${PN}-semodule-expand += "${bindir}/semodule_expand"
FILES_${PN}-semodule-link += "${bindir}/semodule_link"
FILES_${PN}-semodule-package += "\
	${bindir}/semodule_package \
	${bindir}/semodule_unpackage \
"
FILES_${PN}-hll += "${prefix}/libexec/selinux/hll/*"

FILES_${PN}-sepolicy += "\
	${bindir}/sepolicy \
	${datadir}/bash-completion/completions/sepolicy \
	${datadir}/dbus-1/system-services/org.selinux.service \
	${datadir}/polkit-1/actions/org.selinux.policy \
"
FILES_${PN}-sepolgen-ifgen += "\
	${bindir}/sepolgen-ifgen \
	${bindir}/sepolgen-ifgen-attr-helper \
"
FILES_${PN}-sestatus += "\
	${sbindir}/sestatus \
	${sysconfdir}/sestatus.conf \
"
FILES_${PN}-setfiles += "\
	${base_sbindir}/restorecon \
	${base_sbindir}/setfiles \
"
FILES_${PN}-setsebool += "\
	${sbindir}/setsebool \
	${datadir}/bash-completion/completions/setsebool \
"
FILES_mcstrans = "\
	${base_sbindir}/mcstransd \
	${sbindir}/untranscon \
	${sbindir}/transcon \
	${sysconfdir}/init.d/mcstrans \
	${systemd_unitdir}/system/mcstrans.service \
	${sysconfdir}/default/volatiles/volatiles.80_mcstrans \
	${sysconfdir}/tmpfiles.d/setrans.conf \
"

FILES_mcstrans-doc = "\
    /usr/share/man/man8/mcstransd.8 \
    /usr/share/man/man8/mcs.8 \
    /usr/share/man/man8/setrans.conf.8 \
    ${datadir}/mcstrans \
"

FILES_system-config-selinux = " \
    ${bindir}/sepolgen \
    ${datadir}/system-config-selinux/* \
    ${datadir}/icons/hicolor/ \
    ${datadir}/polkit-1/actions/org.selinux.config.policy \
"

export STAGING_INCDIR
export STAGING_LIBDIR
export BUILD_SYS
export HOST_SYS

AUDITH="`ls ${STAGING_INCDIR}/libaudit.h >/dev/null 2>&1 && echo /usr/include/libaudit.h `"
PAMH="`ls ${STAGING_INCDIR}/security/pam_appl.h >/dev/null 2>&1 && echo /usr/include/security/pam_appl.h `"
EXTRA_OEMAKE += "${@target_selinux(d, 'PAMH=${PAMH} AUDITH=${AUDITH}', 'PAMH= AUDITH= ')} INOTIFYH=n"
EXTRA_OEMAKE += "PREFIX=${D}"
EXTRA_OEMAKE += "INITDIR=${D}/etc/init.d"

BBCLASSEXTEND = "native"

PCU_NATIVE_CMDS = "setfiles semodule_package semodule semodule_link semodule_expand semodule_deps hll"

do_compile_virtclass-native() {
	for PCU_CMD in ${PCU_NATIVE_CMDS} ; do
		oe_runmake -C $PCU_CMD \
			INCLUDEDIR='${STAGING_INCDIR}' \
			LIBDIR='${STAGING_LIBDIR}'
	done
}

sysroot_stage_dirs_append_virtclass-native() {
	cp -R $from/${prefix}/libexec $to/${prefix}/libexec
}

do_compile_prepend() {
	export PYTHON=python
	export PYLIBVER='python${PYTHON_BASEVERSION}'
	export PYTHON_CPPFLAGS="-I${STAGING_INCDIR}/${PYLIBVER}"
	export PYTHON_LDFLAGS="${STAGING_LIBDIR}/lib${PYLIBVER}.so"
	export PYTHON_SITE_PKG="${libdir}/${PYLIBVER}/site-packages"
}

do_install_prepend() {
	export PYTHON=python
	export SEMODULE_PATH=${sbindir} SYSTEMDDIR=${D}/${systemd_unitdir}
}

do_install_append_class-target() {
	install -m 755 mcstrans/utils/untranscon ${D}${sbindir}/
	install -m 755 mcstrans/utils/transcon ${D}${sbindir}/

	if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
		install -d ${D}${sysconfdir}/tmpfiles.d
		echo "d ${localstatedir}/run/setrans - - - -" \
		    > ${D}${sysconfdir}/tmpfiles.d/setrans.conf
	else
		install -d ${D}${sysconfdir}/default/volatiles
		echo "d root root 0755 /var/run/setrans none" \
			>${D}${sysconfdir}/default/volatiles/volatiles.80_mcstrans
	fi
	install -d ${D}${datadir}/mcstrans
	cp -r mcstrans/share/* ${D}${datadir}/mcstrans/.
}

do_install_virtclass-native() {
	for PCU_CMD in ${PCU_NATIVE_CMDS} ; do
	     oe_runmake -C $PCU_CMD install \
			DESTDIR="${D}" \
			PREFIX="${D}/${prefix}" \
			SBINDIR="${D}/${base_sbindir}"
	done
}

do_install_append_class-target() {
	if [ -e ${WORKDIR}/pam.d ]; then
		install -d ${D}${sysconfdir}/pam.d/
		install -m 0644 ${WORKDIR}/pam.d/* ${D}${sysconfdir}/pam.d/
	fi

	# /var/lib/selinux is involved by seobject.py:
	#   + dirname = "/var/lib/selinux"
	# and it's required for running command:
	#   $ semanage permissive [OPTS]
	install -d ${D}${localstatedir}/lib/selinux
}

SYSTEMD_SERVICE_mcstrans = "mcstrans.service"
INITSCRIPT_PACKAGES = "mcstrans"
INITSCRIPT_NAME_mcstrans = "mcstrans"
INITSCRIPT_PARAMS_mcstrans = "defaults"

pkg_postinst_mcstrans () {
    if [ -z "$D" ]; then
        if command -v systemd-tmpfiles >/dev/null; then
            systemd-tmpfiles --create ${sysconfdir}/tmpfiles.d/setrans.conf
        elif [ -e ${sysconfdir}/init.d/populate-volatile.sh ]; then
            ${sysconfdir}/init.d/populate-volatile.sh update
        fi
    fi
}
