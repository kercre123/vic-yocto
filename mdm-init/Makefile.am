# Makefile.am - Automake script for device-krypton

etcdir = ${sysconfdir}
dist_etc_DATA = sysctl.conf

ACLOCAL_AMFLAGS = -I m4

if IS_MDM9607
dirs = wlan_sdio
else
if IS_APQ8096_DRONE
dirs = wlan_apq8096_drone
else
if IS_APQ8009_DRONE
dirs = wlan_apq8009_drone
else
if IS_APQ8017_SNAP
dirs = wlan_apq8017_snap
else
if IS_APQ8009_ROBOT
dirs = wlan_apq8009_robot
else
if IS_PRONTO_ENABLED
dirs = wlan_pronto
else
if IS_APQ8098
dirs = wlan_apq8098_mediabox
else
if IS_IPC
dirs = wlan_ipc
else
if IS_MDM9650
dirs = wlan_mdm9650
else
if IS_SDX20
dirs = wlan_sdx20
else
dirs = wlan
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

SUBDIRS = $(dirs)
