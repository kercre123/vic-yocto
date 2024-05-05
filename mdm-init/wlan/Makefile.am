etcdir = ${sysconfdir}
dist_etc_DATA = cld-hostapd-wlan0.sh
dist_etc_DATA += cld-hostapd-wlan1.sh
dist_etc_DATA += cld-hostapd-wlan2.sh

misc_ddir = ${userfsdatadir}/misc/wifi
dist_misc_d_DATA = hostapd.conf
dist_misc_d_DATA += hostapd-wlan1.conf
dist_misc_d_DATA += hostapd-wlan2.conf
dist_misc_d_DATA += sta_mode_hostapd.conf
dist_misc_d_DATA += wpa_supplicant.conf

init_ddir = ${sysconfdir}/init.d
dist_init_d_SCRIPTS = wlan

firmware_ddir = ${base_libdir}/firmware/wlan/qca_cld
dist_firmware_d_DATA = WCNSS_qcom_cfg.ini

if NEED_SDIO_INI

# Copy WCNSS_qcom_cfg.sdio.in to ../firmware/wlan/qca_cld/qca9377/
firmware_qca9377dir = ${base_libdir}/firmware/wlan/qca_cld/qca9377
dist_firmware_qca9377_DATA = WCNSS_qcom_cfg.sdio.ini

# Rename WCNSS_qcom_cfg.sdio.ini to WCNSS_qcom_cfg.ini
install-data-hook:$(DESTDIR)$(firmware_qca9377dir)/WCNSS_qcom_cfg.sdio.ini
	cd $(DESTDIR)$(firmware_qca9377dir) && \
		mv -f WCNSS_qcom_cfg.sdio.ini WCNSS_qcom_cfg.ini

endif
