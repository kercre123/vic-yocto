# Data OPEN SOURCE PACKAGES
include ${BASEMACHINE}/${BASEMACHINE}-data-image.inc

PACKAGE_EXCLUDE_mdm9607 += "avahi-systemd avahi-daemon avahi-dnsconfd avahi-autoipd avahi-utils libavahi-common libavahi-core libavahi-client libavahi-glib dnsmasq lighttpd minidlna miniupnpd rtsp-alg iputils"
