DESCRIPTION = "WCNSS platform"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
LICENSE = "BSD"

PR = "r1"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://qcom-opensource/wlan/prima/firmware_bin \
           file://set_wcnss_mode"
SRC_URI += "file://wcnss_wlan.service"

S = "${WORKDIR}/qcom-opensource/wlan/firmware_bin"

inherit systemd
inherit update-rc.d

do_install() {

	if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
                install -d ${D}/etc/initscripts/
                install "${WORKDIR}"/set_wcnss_mode ${D}/etc/initscripts/set_wcnss_mode
		install -d ${D}/etc/systemd/system/
		install -m 0644 ${WORKDIR}/wcnss_wlan.service -D ${D}/etc/systemd/system/wcnss_wlan.service
	        install -d ${D}/etc/systemd/system/multi-user.target.wants/
	        install -d ${D}/etc/systemd/system/ffbm.target.wants/
		# enable the service for multi-user.target
		ln -sf /etc/systemd/system/wcnss_wlan.service \
		${D}/etc/systemd/system/multi-user.target.wants/wcnss_wlan.service
		# enable the service for ffbm.target, ffbm is used for factory
		ln -sf /etc/systemd/system/wcnss_wlan.service \
		${D}/etc/systemd/system/ffbm.target.wants/wcnss_wlan.service
        else
               install -d ${D}/etc
               install -d ${D}/etc/init.d
               install "${WORKDIR}"/set_wcnss_mode ${D}/etc/init.d
	fi

    mkdir -p ${D}/lib/firmware/wlan/prima
    cp -pP ${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_qcom_cfg.ini ${D}/lib/firmware/wlan/prima
}
do_install_append() {
   install -d ${D}/lib/firmware/wlan/prima
   if [ -e "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_qcom_wlan_nv.bin" ];then
       cp -rf "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_qcom_wlan_nv.bin" ${D}/lib/firmware/wlan/prima
   fi

   if [ -e "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_wlan_dictionary.dat" ]; then
       cp -rf "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_wlan_dictionary.dat" ${D}/lib/firmware/wlan/prima
   elif [ -e "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}_32/WCNSS_wlan_dictionary.dat" ]; then
       cp -rf "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}_32/WCNSS_wlan_dictionary.dat" ${D}/lib/firmware/wlan/prima
   fi
}
INITSCRIPT_NAME = "set_wcnss_mode"
INITSCRIPT_PARAMS = "start 60 2 3 4 5 . stop 20 0 1 6 ."

FILES_${PN} = "/lib/firmware/*"
FILES_${PN} += "/etc/*"
FILES_${PN} += "/lib/firmware/wlan/prima/*"
