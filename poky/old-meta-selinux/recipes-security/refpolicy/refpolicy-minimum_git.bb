include refpolicy-targeted_${PV}.bb

SUMMARY = "SELinux minimum policy"
DESCRIPTION = "\
This is a minimum reference policy with just core policy modules, and \
could be used as a base for customizing targeted policy. \
Pretty much everything runs as initrc_t or unconfined_t so all of the \
domains are unconfined. \
"

POLICY_NAME = "minimum"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:${THISDIR}/refpolicy-${PV}:${THISDIR}/refpolicy-targeted:"

CORE_POLICY_MODULES = "unconfined \
	selinuxutil storage sysnetwork \
	application libraries miscfiles logging userdomain \
	init mount modutils getty authlogin locallogin \
	"

# nscd caches libc-issued requests to the name service.
# Without nscd.pp, commands want to use these caches will be blocked.
EXTRA_POLICY_MODULES += "nscd"

# pam_mail module enables checking and display of mailbox status upon
# "login", so "login" process will access to /var/spool/mail.
EXTRA_POLICY_MODULES += "mta"

# sysnetwork requires type definitions (insmod_t, consoletype_t,
# hostname_t, ping_t, netutils_t) from modules:
EXTRA_POLICY_MODULES += "modutils consoletype hostname netutils"

POLICY_MODULES_MIN = "${CORE_POLICY_MODULES} ${EXTRA_POLICY_MODULES}"

# re-write the same func from refpolicy_common.inc
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

	for i in base ${POLICY_MODULES_MIN}; do
		MOD_FILE=${POL_SRC}/${i}.${HLL_TYPE}
		MOD_DIR=${POL_ACTIVE_MODS}/${i}
		mkdir -p ${MOD_DIR}
		echo -n "${HLL_TYPE}" > ${MOD_DIR}/lang_ext

		if ! bzip2 -t ${MOD_FILE} >/dev/null 2>&1; then
			${HLL_BIN} ${MOD_FILE} | bzip2 --stdout > ${MOD_DIR}/cil
			bzip2 -f ${MOD_FILE} && mv -f ${MOD_FILE}.bz2 ${MOD_FILE}
		else
			bunzip2 --stdout ${MOD_FILE} | \
				${HLL_BIN} | \
				bzip2 --stdout > ${MOD_DIR}/cil
		fi
		cp ${MOD_FILE} ${MOD_DIR}/hll
	done
}
