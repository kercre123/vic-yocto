FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://Disable-unused-mount-points.patch"
SRC_URI += "file://mountpartitions.rules"
SRC_URI += "file://systemd-udevd.service"
SRC_URI += "file://ffbm.target"
#SRC_URI += "file://mtpserver.rules"
SRC_URI += "file://ion.rules"
SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', [ 'ab-boot-support', 'nand-ab' ], 'file://mtd.rules', '', d)}"
SRC_URI += "file://kgsl.rules"
SRC_URI += "file://set-usb-nodes.rules"
SRC_URI += "file://sysctl.conf"
SRC_URI += "file://platform.conf"
SRC_URI += "file://sd-bus-Allow-extra-users-to-communicate.patch"
SRC_URI += "file://systemd-namespace-mountflags-fix.patch"
SRC_URI += "file://set-mhi-nodes.rules"
SRC_URI += "file://preventing-creation-of-rd-dir.patch"

# Custom setup for PACKAGECONFIG to get a slimmer systemd.
# Removed following:
#   * backlight - Loads/Saves Screen Backlight Brightness, not required.
#   * firstboot - initializes the most basic system settings interactively
#                  on the first boot if /etc is empty, not required.
#   * hostname  - No need to change the system's hostname
#   * ldconfig  - configures dynamic linker run-time bindings.
#                 ldconfig  creates  the  necessary links and cache to the most
#                 recent shared libraries found in the directories specified on
#                 the command line, in the file /etc/ld.so.conf, and in the
#                 trusted directories (/lib and /usr/lib).  The cache (created
#                 at /etc/ld.so.cache) is used by the run-time linker ld-linux.so.
#                 system-ldconfig.service runs "ldconfig -X", but as / is RO
#                 cache may not be created. Disabling this may introduce app
#                 start time latency.
#   * localed   - Service used to change the system locale settings, not needed.
#   * machined  - For tracking local Virtual Machines and Containers, not needed.
#   * networkd  - Manages network configurations, custom solution is used.
#   * polkit    - Not used.
#   * quotacheck- Not using Quota.
#   * resolvd   - Use custom network name resolution manager.
#   * smack     - Not used.
#   * timesyncd - Chronyd is being used instead for NTP timesync.
#                 Also timesyncd was resulting in higher boot KPI.
#   * utmp      - No back fill for SysV runlevel changes needed.
#   * vconsole  - Not used.
PACKAGECONFIG = " \
    ${@bb.utils.filter('DISTRO_FEATURES', 'selinux', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wifi', 'rfkill', '', d)} \
    acl \
    binfmt \
    hibernate \
    hostnamed \
    ima \
    ${@bb.utils.contains('DISTRO_NAME', 'mdm', '', 'kmod', d)} \
    logind \
    randomseed \
    sysusers \
    timedated \
    xz \
"

# Enable coredump support when needed
PACKAGECONFIG_append = " ${@bb.utils.contains('SYSTEMD_ENABLE_COREDUMP', '1', 'coredump', '', d)}"

EXTRA_OEMESON += " -Defi=false"
EXTRA_OEMESON += " -Dhwdb=false"

CFLAGS_append = " -fPIC"

# In aarch64 targets systemd is not booting with -finline-functions -finline-limit=64 optimizations
# So temporarily revert to default optimizations for systemd.
SELECTED_OPTIMIZATION = "-O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer -ftree-vectorize"

MACHINE_SUPPORT_BLOCK_DEVICES = "${@bb.utils.contains('IMAGE_FSTYPES','ext4', 'true', 'false', d)}"

do_install_append () {
   install -d ${D}/etc/systemd/system/
   install -d ${D}/lib/systemd/system/ffbm.target.wants
   install -d ${D}/etc/systemd/system/ffbm.target.wants
   rm ${D}/lib/udev/rules.d/60-persistent-v4l.rules

   # Place systemd-udevd.service in /etc/systemd/system
   install -m 0644 ${WORKDIR}/systemd-udevd.service \
       -D ${D}/etc/systemd/system/systemd-udevd.service
   install -m 0644 ${WORKDIR}/ffbm.target \
       -D ${D}/etc/systemd/system/ffbm.target

   # Enable logind/getty/password-wall service in FFBM mode
   ln -sf /lib/systemd/system/systemd-logind.service ${D}/lib/systemd/system/ffbm.target.wants/systemd-logind.service
   ln -sf /lib/systemd/system/getty.target ${D}/lib/systemd/system/ffbm.target.wants/getty.target
   ln -sf /lib/systemd/system/systemd-ask-password-wall.path ${D}/lib/systemd/system/ffbm.target.wants/systemd-ask-password-wall.path
   install -d /etc/sysctl.d/
   install -m 0644 ${WORKDIR}/sysctl.conf -D ${D}/etc/sysctl.d/sysctl.conf
   install -m 0644 ${WORKDIR}/platform.conf -D ${D}/etc/tmpfiles.d/platform.conf

   #  Mask journaling services by default.
   #  'systemctl unmask' can be used on device to enable them if needed.
   #ln -sf /dev/null ${D}/etc/systemd/system/systemd-journald.service
   #ln -sf /dev/null ${D}${systemd_unitdir}/system/sysinit.target.wants/systemd-journal-flush.service
   #ln -sf /dev/null ${D}${systemd_unitdir}/system/sysinit.target.wants/systemd-journal-catalog-update.service
   #if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd-minimal', 'true', 'false', d)}; then
   #    ln -sf /dev/null ${D}${systemd_unitdir}/system/sockets.target.wants/systemd-journald-audit.socket
   #    ln -sf /dev/null ${D}${systemd_unitdir}/system/sockets.target.wants/systemd-journald-dev-log.socket
   #    ln -sf /dev/null ${D}${systemd_unitdir}/system/sockets.target.wants/systemd-journald.socket
   #fi
   install -d ${D}${sysconfdir}/udev/rules.d/
   install -m 0644 ${WORKDIR}/ion.rules -D ${D}${sysconfdir}/udev/rules.d/ion.rules
   if ${@bb.utils.contains('DISTRO_FEATURES', [ 'ab-boot-support', 'nand-ab' ], 'true', 'false', d)}; then
           install -m 0644 ${WORKDIR}/mtd.rules -D ${D}${sysconfdir}/udev/rules.d/mtd.rules
   fi
   install -m 0644 ${WORKDIR}/kgsl.rules -D ${D}${sysconfdir}/udev/rules.d/kgsl.rules
   install -m 0644 ${WORKDIR}/set-mhi-nodes.rules -D ${D}${sysconfdir}/udev/rules.d/set-mhi-nodes.rules

   # Mask dev-ttyS0.device
   ln -sf /dev/null ${D}/etc/systemd/system/dev-ttyS0.device

   # Remove rules to automount block devices.
   if [ "${MACHINE_SUPPORT_BLOCK_DEVICES}" == "false" ]; then
       sed -i '/SUBSYSTEM=="block", TAG+="systemd"/d' ${D}/lib/udev/rules.d/99-systemd.rules
       sed -i '/SUBSYSTEM=="block", ACTION=="add", ENV{DM_UDEV_DISABLE_OTHER_RULES_FLAG}=="1", ENV{SYSTEMD_READY}="0"/d' ${D}/lib/udev/rules.d/99-systemd.rules

       # Remove generator binaries and ensure that we don't rely on generators for mount or service files.
       rm -rf ${D}/lib/systemd/system-generators/systemd-debug-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-fstab-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-getty-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-gpt-auto-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-hibernate-resume-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-rc-local-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-system-update-generator
       rm -rf ${D}/lib/systemd/system-generators/systemd-sysv-generator

       # Start systemd-udev-trigger.service after sysinit.target
       if ${@bb.utils.contains_any('DISTRO_NAME','mdm auto nad-core', 'true', 'false', d)}; then
           sed -i '/Before=sysinit.target/a After=sysinit.target init_sys_mss.service' ${D}${systemd_unitdir}/system/systemd-udev-trigger.service
           sed -i '/Before=sysinit.target/d' ${D}${systemd_unitdir}/system/systemd-udev-trigger.service
       fi
   fi

   if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd-minimal', 'true', 'false', d)}; then
       rm -rf ${D}/usr/lib/tmpfiles.d/*
       rm -rf ${D}${sysconfdir}/udev/rules.d/mtpserver.rules
#Required for soundcard
       cp ${D}/lib/udev/rules.d/50-udev-default.rules  ${D}${sysconfdir}/udev/rules.d/50-udev-default.rules
       if ${@bb.utils.contains('IMAGE_FSTYPES','ext4', 'true', 'false', d)}; then
        cp ${D}/lib/udev/rules.d/60-persistent-storage.rules ${D}${sysconfdir}/udev/rules.d/60-persistent-storage.rules
        cp ${D}/lib/udev/rules.d/99-systemd.rules ${D}${sysconfdir}/udev/rules.d/99-systemd.rules
       fi
       rm -rf ${D}/lib/udev/rules.d/*
   fi
}
# Run fsck as part of local-fs-pre.target instead of local-fs.target
do_install_append () {
   # remove from After
   sed -i '/After/s/local-fs-pre.target//' ${D}${systemd_unitdir}/system/systemd-fsck@.service
   # Add to Before
   sed -i '/Before/s/$/ local-fs-pre.target/' ${D}${systemd_unitdir}/system/systemd-fsck@.service
}

RRECOMMENDS_${PN}_remove += "systemd-extra-utils"
PACKAGES_remove += "${PN}-extra-utils"

FILES_${PN} += "/etc/initscripts \
                ${sysconfdir}/udev/rules.d ${userfsdatadir}/*"
