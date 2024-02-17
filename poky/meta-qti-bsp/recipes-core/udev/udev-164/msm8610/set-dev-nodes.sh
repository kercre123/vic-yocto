#!/bin/sh
#
# Called from udev
#
# Attempt to set appropriate permission for all devices node.

# the DIAG device node is not world writable/readable.
chmod 0660 /dev/diag
chmod 0666 /dev/genlock
chmod 0666 /dev/kmem
chmod 0666 /dev/kgsl
chmod 0666 /dev/kgsl-3d0
chmod 0666 /dev/kgsl-2d0
chmod 0666 /dev/kgsl-2d1
chmod 0666 /dev/ion
chmod 0600 /dev/rtc0
#chmod 0660 /dev/smd0
#chmod 0660 /dev/smd1
chmod 0660 /dev/smd4
chmod 0640 /dev/smd_cxm_qmi
chmod 0660 /dev/smd5
chmod 0660 /dev/smd6
chmod 0660 /dev/smd7

#permissions for CSVT
chmod 0660 /dev/smd11

chmod 0640 /dev/radio0
#chmod 0660 /dev/rfcomm0
chmod 0640 /dev/smdcntl0
chmod 0640 /dev/smdcntl1
chmod 0640 /dev/smdcntl2
chmod 0640 /dev/smdcntl3
chmod 0640 /dev/smdcntl4
chmod 0640 /dev/smdcntl5
chmod 0640 /dev/smdcntl6
chmod 0640 /dev/smdcntl7
#chmod 0640 /dev/smdcntl8
#chmod 0640 /dev/smuxctl32
#chmod 0640 /dev/sdioctl0
#chmod 0640 /dev/sdioctl1
#chmod 0640 /dev/sdioctl2
#chmod 0640 /dev/sdioctl3
#chmod 0640 /dev/sdioctl4
#chmod 0640 /dev/sdioctl5
#chmod 0640 /dev/sdioctl6
#chmod 0640 /dev/sdioctl7
#chmod 0640 /dev/sdioctl8
chmod 0640 /dev/rmnet_ctrl
chmod 0640 /dev/rmnet_mux_ctrl
#chmod 0640 /dev/hsicctl0
#chmod 0640 /dev/hsicctl1
#chmod 0640 /dev/hsicctl2
#chmod 0640 /dev/hsicctl3
chmod 0660 /dev/video*
chmod 0660 /dev/vc*
chmod 0660 /dev/media*
chmod 0660 /dev/v4l-subdev*
chmod 0666 /dev/qseecom
#chmod 0660 /dev/gemini0
chmod 0660 /dev/jpeg0
chmod 0660 /dev/jpeg1
chmod 0660 /dev/jpeg2
chmod 0660 /dev/msm_camera/*
chmod 0660 /dev/gemini/
#chmod 0660 /dev/mercury0
#chmod 0660 /dev/msm_vidc_reg
#chmod 0660 /dev/msm_vidc_dec
#chmod 0660 /dev/msm_vidc_dec_sec
#chmod 0660 /dev/msm_vidc_enc
#chmod 0660 /dev/msm_rotator
#chmod 0660 /dev/hw_random
#chmod 0664 /dev/adsprpc-smd

#permissions for audio
chmod 0660 /dev/msm_qcelp
chmod 0660 /dev/msm_evrc
chmod 0660 /dev/msm_wma
chmod 0660 /dev/msm_wmapro
chmod 0660 /dev/msm_amrnb
chmod 0660 /dev/msm_amrwb
chmod 0660 /dev/msm_amrwbplus
chmod 0660 /dev/msm_aac
chmod 0660 /dev/msm_multi_aac
chmod 0660 /dev/msm_aac_in
chmod 0660 /dev/msm_qcelp_in
chmod 0660 /dev/msm_evrc_in
chmod 0640 /dev/msm_amrnb_in
chmod 0640 /dev/msm_amrwb_in
#chmod 0660 /dev/msm_a2dp_in
#chmod 0660 /dev/msm_ac3
chmod 0660 /dev/msm_mp3
chmod 0660 /dev/msm_acdb
chmod 0660 /dev/msm_cad
#chmod 0660 /dev/msm_fm
#chmod 0660 /dev/msm_mvs
#chmod 0660 /dev/msm_pcm_lp_dec
#chmod 0660 /dev/msm_preproc_ctl
chmod 0660 /dev/msm_rtac
chmod 0660 /dev/msm_sps
#chmod 0660 /dev/msm_voicememo
chmod 0640 /dev/radio0
chmod 0660 /dev/smd3
chmod 0660 /dev/smd2
#chmod 0660 /dev/ttyHSL1
#chmod 0660 /dev/mdm
#chmod 0664  /sys/devices/virtual/smdpkt/smdcntl*
chmod 0660 /dev/sdio_tty_ciq_00
chmod 0660 /dev/tty_sdio_00
chmod 0660 /dev/ttyGS0
#chmod 0660 /dev/i2c-5
chmod 0660 /dev/i2c-0
chmod 0660 /dev/i2c-2

# DVB devices
#chmod 0444 /dev/dvb/adapter0/demux*
#chmod 0664 /dev/dvb/adapter0/dvr*
#chmod 0664 /dev/dvb/adapter0/video*

# sensors
#chmod 0664 /sys/devices/i2c-12/12-*

# wlan
chmod 0660  /dev/wcnss_wlan

chmod 0660  /dev/xt_qtaguid
