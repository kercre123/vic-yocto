#!/bin/sh

DUMP_TO_KMSG=/dev/kmsg
flag_file="/data/misc/wifi/FIRST_BOOT.flag"

mac=`cat /sys/class/net/wlan0/address`
var1=`echo $mac|cut -d: -f4| tr '[a-z]' '[A-Z]'`
var2=`echo $mac|cut -d: -f5| tr '[a-z]' '[A-Z]'`
var3=`echo $mac|cut -d: -f6| tr '[a-z]' '[A-Z]'`
ssid=${var1}${var2}${var3}

if [ -f "$flag_file" ]; then
    rm $flag_file
    sed -i "s/^ssid=.*/ssid=IPC8053_$ssid/" /data/misc/wifi/hostapd.conf
    sync
fi

/usr/sbin/hostapd -B -P /var/run/hostapd.wlan0.pid /data/misc/wifi/hostapd.conf

/sbin/brctl addbr br0
ifconfig eth0 down
/sbin/brctl addif br0 wlan0
/sbin/brctl addif br0 eth0
ifconfig eth0 up
ifconfig wlan0 up
ifconfig br0 hw ether $mac up

/sbin/ip -4 monitor addr dev br0 | /usr/bin/ipmon &
#for discovery tcp connect bug
sleep 1
/usr/bin/discovery IPC8053_$ssid &
/usr/bin/ipc-webserver &
/usr/bin/rebootkey &

echo "IPC8053 boot completed" > $DUMP_TO_KMSG

exit 0
