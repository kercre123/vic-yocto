DEFAULT_ENFORCING ??= "enforcing"

SECTION = "base"
LICENSE = "GPLv2"

LIC_FILES_CHKSUM = "file://${S}/COPYING;md5=393a5ca445f6965873eca0259a17f833"

PROVIDES += "virtual/refpolicy"
RPROVIDES_${PN} += "refpolicy"

# Specific config files for Poky
SRC_URI += "file://customizable_types \
            file://setrans-mls.conf \
            file://setrans-mcs.conf \
	   "

S = "${WORKDIR}/refpolicy"

CONFFILES_${PN} += "${sysconfdir}/selinux/config"
FILES_${PN} += " \
	${sysconfdir}/selinux/${POLICY_NAME}/ \
	${datadir}/selinux/${POLICY_NAME}/*.pp \
	${localstatedir}/lib/selinux/${POLICY_NAME}/ \
	"
FILES_${PN}-dev =+ " \
        ${datadir}/selinux/${POLICY_NAME}/include/ \
        ${sysconfdir}/selinux/sepolgen.conf \
"

EXTRANATIVEPATH += "bzip2-native"

DEPENDS += "bzip2-replacement-native checkpolicy-native policycoreutils-native semodule-utils-native m4-native"

RDEPENDS_${PN}-dev =+ " \
        python \
"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit pythonnative

PARALLEL_MAKE = ""

POLICY_NAME ?= "${POLICY_TYPE}"
POLICY_DISTRO ?= "redhat"
POLICY_UBAC ?= "n"
POLICY_UNK_PERMS ?= "allow"
POLICY_DIRECT_INITRC ?= "n"
POLICY_SYSTEMD ?= "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'y', 'n', d)}"
POLICY_MONOLITHIC ?= "n"
POLICY_CUSTOM_BUILDOPT ?= ""
POLICY_QUIET ?= "y"
POLICY_MLS_SENS ?= "16"
POLICY_MLS_CATS ?= "1024"
POLICY_MCS_CATS ?= "1024"

EXTRA_OEMAKE += "NAME=${POLICY_NAME} \
	TYPE=${POLICY_TYPE} \
	DISTRO=${POLICY_DISTRO} \
	UBAC=${POLICY_UBAC} \
	UNK_PERMS=${POLICY_UNK_PERMS} \
	DIRECT_INITRC=${POLICY_DIRECT_INITRC} \
	SYSTEMD=${POLICY_SYSTEMD} \
	MONOLITHIC=${POLICY_MONOLITHIC} \
	CUSTOM_BUILDOPT=${POLICY_CUSTOM_BUILDOPT} \
	QUIET=${POLICY_QUIET} \
	MLS_SENS=${POLICY_MLS_SENS} \
	MLS_CATS=${POLICY_MLS_CATS} \
	MCS_CATS=${POLICY_MCS_CATS}"

EXTRA_OEMAKE += "tc_usrbindir=${STAGING_BINDIR_NATIVE}"
EXTRA_OEMAKE += "OUTPUT_POLICY=`${STAGING_BINDIR_NATIVE}/checkpolicy -V | cut -d' ' -f1`"
EXTRA_OEMAKE += "CC='${BUILD_CC}' CFLAGS='${BUILD_CFLAGS}' PYTHON='${PYTHON}'"

python __anonymous () {
    import re

    # make sure DEFAULT_ENFORCING is something sane
    if not re.match('^(enforcing|permissive|disabled)$',
                    d.getVar('DEFAULT_ENFORCING', True),
                    flags=0):
        d.setVar('DEFAULT_ENFORCING', 'permissive')
}

do_compile() {
	oe_runmake conf
	oe_runmake policy
}

prepare_policy_store () {
	oe_runmake 'DESTDIR=${D}' 'prefix=${D}${prefix}' install
	POL_PRIORITY=100
	POL_SRC=${D}${datadir}/selinux/${POLICY_NAME}
	POL_STORE=${D}${localstatedir}/lib/selinux/${POLICY_NAME}
	POL_ACTIVE_MODS=${POL_STORE}/active/modules/${POL_PRIORITY}

	# Prepare to create policy store
	mkdir -p ${POL_STORE}
	mkdir -p ${POL_ACTIVE_MODS}

	# get hll type from suffix on base policy module
	HLL_TYPE=$(echo ${POL_SRC}/base.* | awk -F . '{if (NF>1) {print $NF}}')
	HLL_BIN=${STAGING_DIR_NATIVE}${prefix}/libexec/selinux/hll/${HLL_TYPE}

	for i in ${POL_SRC}/*.${HLL_TYPE}; do
		MOD_NAME=$(basename $i | sed "s/\.${HLL_TYPE}$//")
		MOD_DIR=${POL_ACTIVE_MODS}/${MOD_NAME}
		mkdir -p ${MOD_DIR}
		echo -n "${HLL_TYPE}" > ${MOD_DIR}/lang_ext
		if ! bzip2 -t $i >/dev/null 2>&1; then
			${HLL_BIN} $i | bzip2 --stdout > ${MOD_DIR}/cil
			bzip2 -f $i && mv -f $i.bz2 $i
		else
			bunzip2 --stdout $i | \
				${HLL_BIN} | \
				bzip2 --stdout > ${MOD_DIR}/cil
		fi
		cp $i ${MOD_DIR}/hll
	done
}

rebuild_policy () {
	cat <<-EOF > ${D}${sysconfdir}/selinux/semanage.conf
module-store = direct
[setfiles]
path = ${STAGING_DIR_NATIVE}${base_sbindir_native}/setfiles
args = -q -c \$@ \$<
[end]
[sefcontext_compile]
path = ${STAGING_DIR_NATIVE}${sbindir_native}/sefcontext_compile
args = \$@
[end]

policy-version = 30
EOF

	# Create policy store and build the policy
	semodule -p ${D} -s ${POLICY_NAME} -n -B
	rm -f ${D}${sysconfdir}/selinux/semanage.conf
	# no need to leave final dir created by semanage laying around
	rm -rf ${D}${localstatedir}/lib/selinux/final
}

install_misc_files () {
	cat ${WORKDIR}/customizable_types >> \
		${D}${sysconfdir}/selinux/${POLICY_NAME}/contexts/customizable_types

	# install setrans.conf for mls/mcs policy
	if [ -f ${WORKDIR}/setrans-${POLICY_TYPE}.conf ]; then
		install -m 0644 ${WORKDIR}/setrans-${POLICY_TYPE}.conf \
			${D}${sysconfdir}/selinux/${POLICY_NAME}/setrans.conf
	fi

	# install policy headers
	oe_runmake 'DESTDIR=${D}' 'prefix=${D}${prefix}' install-headers
}

install_config () {
	echo "\
# This file controls the state of SELinux on the system.
# SELINUX= can take one of these three values:
#     enforcing - SELinux security policy is enforced.
#     permissive - SELinux prints warnings instead of enforcing.
#     disabled - No SELinux policy is loaded.
SELINUX=${DEFAULT_ENFORCING}
# SELINUXTYPE= can take one of these values:
#     minimum - Minimum Security protection.
#     standard - Standard Security protection.
#     mls - Multi Level Security protection.
#     targeted - Targeted processes are protected.
#     mcs - Multi Category Security protection.
SELINUXTYPE=${POLICY_NAME}
" > ${WORKDIR}/config
	install -d ${D}/${sysconfdir}/selinux
	install -m 0644 ${WORKDIR}/config ${D}/${sysconfdir}/selinux/
}

do_install () {
	prepare_policy_store
	rebuild_policy
	install_misc_files
	install_config
}

do_install_append(){
	# While building policies on target, Makefile will be searched from SELINUX_DEVEL_PATH
	echo "SELINUX_DEVEL_PATH=${datadir}/selinux/${POLICY_NAME}/include" > ${D}${sysconfdir}/selinux/sepolgen.conf
}

sysroot_stage_all_append () {
	sysroot_stage_dir ${D}${sysconfdir} ${SYSROOT_DESTDIR}${sysconfdir}
}
