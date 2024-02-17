inherit autotools pkgconfig systemd update-rc.d qperf

DESCRIPTION = "Android system/core components"
HOMEPAGE = "http://developer.android.com/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI   = "file://system/core/"

SRC_URI += "file://init_post_boot.diff"

S = "${WORKDIR}/system/core"
PR = "r19"

DEPENDS = "virtual/kernel openssl glib-2.0 libselinux safe-iop ext4-utils libunwind libcutils libmincrypt"

EXTRA_OECONF = " --with-host-os=${HOST_OS} --with-glib"
EXTRA_OECONF_append = " --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"
EXTRA_OECONF_append = " --with-logd-logging"

# Disable adb root privileges in USER builds for msm targets
EXTRA_OECONF_append_msm = "${@base_conditional('USER_BUILD','1',' --disable-adb-root','',d)}"

CPPFLAGS += "-I${STAGING_INCDIR}/ext4_utils"
CPPFLAGS += "-I${STAGING_INCDIR}/libselinux"
CPPFLAGS += "-I${STAGING_INCDIR}/libunwind"

CPPFLAGS_append_apq8053 += " -DTARGET_IS_64_BIT"
CPPFLAGS_append_apq8017 += " -DTARGET_IS_64_BIT"
CPPFLAGS_append_apq8096 += " -DTARGET_IS_64_BIT"
COMPOSITION         = "9025"
COMPOSITION_apq8009 = "9091"
COMPOSITION_apq8053 = "901D"
COMPOSITION_apq8096 = "901D"
COMPOSITION_apq8098 = "901D"

do_install_append() {
   install -m 0755 ${S}/usb/usb_composition -D ${D}${base_sbindir}/
   install -d ${D}${base_sbindir}/usb/compositions/
   install -m 0755 ${S}/usb/compositions/* -D ${D}${base_sbindir}/usb/compositions/
   install -d ${D}${userfsdatadir}/usb/
   install -m 0755 ${S}/usb/compositions/hsic_next -D ${D}${userfsdatadir}/usb/
   install -m 0755 ${S}/usb/compositions/hsusb_next -D ${D}${userfsdatadir}/usb/
   rm ${S}/usb/compositions/hsic_next
   rm ${S}/usb/compositions/hsusb_next
   install -m 0755 ${S}/usb/target -D ${D}${base_sbindir}/usb/
   install -d ${D}${base_sbindir}/usb/debuger/
   install -m 0755 ${S}/usb/debuger/debugFiles -D ${D}${base_sbindir}/usb/debuger/
   install -m 0755 ${S}/usb/debuger/help -D ${D}${base_sbindir}/usb/debuger/
   install -m 0755 ${S}/usb/debuger/usb_debug -D ${D}${base_sbindir}/
   ln -s  /sbin/usb/compositions/${COMPOSITION} ${D}${userfsdatadir}/usb/boot_hsusb_composition
   ln -s  /sbin/usb/compositions/empty ${D}${userfsdatadir}/usb/boot_hsic_composition
   if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
      install -m 0755 ${S}/logd/start_logd -D ${D}${sysconfdir}/initscripts/logd
      install -m 0755 ${S}/usb/start_usb -D ${D}${sysconfdir}/initscripts/usb
      install -m 0755 ${S}/rootdir/etc/init.qcom.post_boot.sh -D ${D}${sysconfdir}/initscripts/init_post_boot
      install -d ${D}${systemd_unitdir}/system/
      install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
      install -d ${D}${systemd_unitdir}/system/ffbm.target.wants/
      if [[ ${USER_BUILD} != "1" ]]; then
        install -m 0644 ${S}/adb/adbd.service -D ${D}${systemd_unitdir}/system/adbd.service
        ln -sf ${systemd_unitdir}/system/adbd.service ${D}${systemd_unitdir}/system/multi-user.target.wants/adbd.service
        ln -sf ${systemd_unitdir}/system/adbd.service ${D}${systemd_unitdir}/system/ffbm.target.wants/adbd.service
      fi
      install -m 0644 ${S}/logd/logd.service -D ${D}${systemd_unitdir}/system/logd.service
      ln -sf ${systemd_unitdir}/system/logd.service ${D}${systemd_unitdir}/system/multi-user.target.wants/logd.service
      ln -sf ${systemd_unitdir}/system/logd.service ${D}${systemd_unitdir}/system/ffbm.target.wants/logd.service
      install -m 0644 ${S}/usb/usb.service -D ${D}${systemd_unitdir}/system/usb.service
      ln -sf ${systemd_unitdir}/system/usb.service ${D}${systemd_unitdir}/system/multi-user.target.wants/usb.service
      ln -sf ${systemd_unitdir}/system/usb.service ${D}${systemd_unitdir}/system/ffbm.target.wants/usb.service
      install -m 0644 ${S}/rootdir/etc/init_post_boot.service -D ${D}${systemd_unitdir}/system/init_post_boot.service
      ln -sf ${systemd_unitdir}/system/init_post_boot.service \
          ${D}${systemd_unitdir}/system/multi-user.target.wants/init_post_boot.service
      ln -sf ${systemd_unitdir}/system/init_post_boot.service \
          ${D}${systemd_unitdir}/system/ffbm.target.wants/init_post_boot.service
      install -m 0644 ${S}/debuggerd/init_debuggerd.service -D ${D}${systemd_unitdir}/system/init_debuggerd.service
      ln -sf ${systemd_unitdir}/system/init_debuggerd.service \
          ${D}${systemd_unitdir}/system/multi-user.target.wants/init_debuggerd.service
      ln -sf ${systemd_unitdir}/system/init_debuggerd.service \
          ${D}${systemd_unitdir}/system/ffbm.target.wants/init_debuggerd.service
      install -m 0644 ${S}/leproperties/leprop.service -D ${D}${systemd_unitdir}/system/leprop.service
      ln -sf ${systemd_unitdir}/system/leprop.service \
          ${D}${systemd_unitdir}/system/multi-user.target.wants/leprop.service
      ln -sf ${systemd_unitdir}/system/leprop.service \
          ${D}${systemd_unitdir}/system/ffbm.target.wants/leprop.service
      if ${@bb.utils.contains('DISTRO_FEATURES', 'wifi', 'true', 'false', d)}; then
          cat >> ${D}${sysconfdir}/initscripts/init_post_boot <<EOT

export MODULE_BASE=/lib/modules/\`uname -r\`
case "\$1" in
start)
    insmod \$MODULE_BASE/extra/wlan.ko
esac
EOT
      fi
   else
      install -m 0755 ${S}/logd/start_logd -D ${D}${sysconfdir}/init.d/logd
      install -m 0755 ${S}/usb/start_usb -D ${D}${sysconfdir}/init.d/usb
      install -m 0755 ${S}/rootdir/etc/init.qcom.post_boot.sh -D ${D}${sysconfdir}/init.d/init_post_boot
   fi
}

do_install_append_apq8009() {
   if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
    install -m 0755 ${S}/debuggerd/start_debuggerd -D ${D}${sysconfdir}/init.d/init_debuggerd
   else
    install -m 0755 ${S}/debuggerd/start_debuggerd -D ${D}${sysconfdir}/initscripts/init_debuggerd
   fi
}
do_install_append_apq8053(){
  if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
   install -m 0755 ${S}/debuggerd/start_debuggerd64 -D ${D}${sysconfdir}/init.d/init_debuggerd
  else
   install -m 0755 ${S}/debuggerd/start_debuggerd64 -D ${D}${sysconfdir}/initscripts/init_debuggerd
  fi
}
do_install_append_apq8096() {
  if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
    install -m 0755 ${S}/debuggerd/start_debuggerd64 -D ${D}${sysconfdir}/init.d/init_debuggerd
  else
    install -m 0755 ${S}/debuggerd/start_debuggerd64 -D ${D}${sysconfdir}/initscripts/init_debuggerd
  fi
}
do_install_append_apq8017(){
  if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'false', 'true', d)}; then
   install -m 0755 ${S}/debuggerd/start_debuggerd64 -D ${D}${sysconfdir}/init.d/init_debuggerd
  else
   install -m 0755 ${S}/debuggerd/start_debuggerd64 -D ${D}${sysconfdir}/initscripts/init_debuggerd
  fi
}

INITSCRIPT_PACKAGES =+ "${PN}-usb"
INITSCRIPT_NAME_${PN}-usb = "usb"
INITSCRIPT_PARAMS_${PN}-usb = "start 30 2 3 4 5 ."
INITSCRIPT_PARAMS_${PN}-usb_mdm = "start 30 S ."

INITSCRIPT_PACKAGES =+ "${PN}-debuggerd"
INITSCRIPT_NAME_${PN}-debuggerd = "init_debuggerd"
INITSCRIPT_PARAMS_${PN}-debuggerd = "start 31 2 3 4 5 ."
INITSCRIPT_PARAMS_${PN}-debuggerd += "stop 38 6 ."

INITSCRIPT_PACKAGES =+ "${PN}-logd"
INITSCRIPT_NAME_${PN}-logd = "logd"
INITSCRIPT_PARAMS_${PN}-logd = "start 10  2 3 4 5 ."
INITSCRIPT_PARAMS_${PN}-logd += "stop 39  6 ."

INITSCRIPT_PACKAGES =+ "${PN}-post-boot"
INITSCRIPT_NAME_${PN}-post-boot = "init_post_boot"
INITSCRIPT_PARAMS_${PN}-post-boot = "start 90 2 3 4 5 ."

PACKAGES =+ "${PN}-adbd-dbg ${PN}-adbd ${PN}-adbd-dev"
FILES_${PN}-adbd-dbg = "${base_sbindir}/.debug/adbd ${libdir}/.debug/libadbd.*"
FILES_${PN}-adbd     = "${base_sbindir}/adbd ${sysconfdir}/init.d/adbd ${libdir}/libadbd.so.*"
FILES_${PN}-adbd    += "${systemd_unitdir}/system/adbd.service ${systemd_unitdir}/system/multi-user.target.wants/adbd.service ${systemd_unitdir}/system/ffbm.target.wants/adbd.service ${sysconfdir}/initscripts/adbd"
FILES_${PN}-adbd-dev = "${libdir}/libadbd.so ${libdir}/libadbd.la"

PACKAGES =+ "${PN}-usb-dbg ${PN}-usb"
FILES_${PN}-usb-dbg  = "${bindir}/.debug/usb_composition_switch"
FILES_${PN}-usb      = "${sysconfdir}/init.d/usb ${base_sbindir}/usb_composition ${bindir}/usb_composition_switch ${base_sbindir}/usb/compositions/*"
FILES_${PN}-usb     += "${userfsdatadir}/usb/*"
FILES_${PN}-usb     += "${base_sbindir}/usb/* ${base_sbindir}/usb_debug ${base_sbindir}/usb/debuger/*"
FILES_${PN}-usb     += "${systemd_unitdir}/system/usb.service ${systemd_unitdir}/system/multi-user.target.wants/usb.service ${systemd_unitdir}/system/ffbm.target.wants/usb.service ${sysconfdir}/initscripts/usb"

PACKAGES =+ "${PN}-post-boot"
FILES_${PN}-post-boot  = "${sysconfdir}/init.d/init_post_boot"
FILES_${PN}-post-boot += "${systemd_unitdir}/system/init_post_boot.service ${systemd_unitdir}/system/multi-user.target.wants/init_post_boot.service ${systemd_unitdir}/system/ffbm.target.wants/init_post_boot.service ${sysconfdir}/initscripts/init_post_boot"
INSANE_SKIP_${PN}-post-boot = "file-rdeps"

PACKAGES =+ "${PN}-logd-dbg ${PN}-logd"
FILES_${PN}-logd-dbg  = "${base_sbindir}/.debug/logd"
FILES_${PN}-logd      = "${sysconfdir}/init.d/logd ${base_sbindir}/logd"
FILES_${PN}-logd     += "${systemd_unitdir}/system/logd.service ${systemd_unitdir}/system/multi-user.target.wants/logd.service ${systemd_unitdir}/system/ffbm.target.wants/logd.service ${sysconfdir}/initscripts/logd"

PACKAGES =+ "${PN}-debuggerd-dbg ${PN}-debuggerd"
FILES_${PN}-debuggerd-dbg  = "${base_sbindir}/.debug/debuggerd ${base_sbindir}/.debug/debuggerd64 "
FILES_${PN}-debuggerd      = "${sysconfdir}/init.d/init_debuggerd ${sysconfdir}/initscripts/init_debuggerd ${base_sbindir}/debuggerd ${base_sbindir}/debuggerd64"
FILES_${PN}-debuggerd     += "${systemd_unitdir}/system/init_debuggerd.service ${systemd_unitdir}/system/multi-user.target.wants/init_debuggerd.service ${systemd_unitdir}/system/ffbm.target.wants/init_debuggerd.service"

PACKAGES =+ "${PN}-leprop-dbg ${PN}-leprop"
FILES_${PN}-leprop-dbg  = "${base_sbindir}/.debug/leprop-service ${bindir}/.debug/getprop ${bindir}/.debug/setprop"
FILES_${PN}-leprop      = "${base_sbindir}/leprop-service ${bindir}/getprop ${bindir}/setprop ${sysconfdir}/proptrigger.sh ${sysconfdir}/proptrigger.conf"
FILES_${PN}-leprop     += "${systemd_unitdir}/system/leprop.service ${systemd_unitdir}/system/multi-user.target.wants/leprop.service ${systemd_unitdir}/system/ffbm.target.wants/leprop.service"

FILES_${PN}-dbg  = "${bindir}/.debug/* ${libdir}/.debug/*"
FILES_${PN}      = "${bindir}/* ${libdir}/pkgconfig/* ${libdir}/*.so.* "
FILES_${PN}-dev  = "${libdir}/*.so ${libdir}/*.la ${includedir}*"
