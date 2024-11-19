#!/bin/bash

IMAGEPATH=../poky/build/tmp-glibc/deploy/images/apq8009-robot/machine-robot-image-apq8009-robot.ext4

set -e

if [[ $1 == *"h"* ]]; then
    echo "Usage: ./install.sh vectorip /path/to/sshkey ssid password"
    echo 'Example: ./install.sh 192.168.1.50 /home/kerigan/ssh_root_key "AnkiRobits" "KlaatuBaradaNikto!"'
    exit 0
fi

if [[ "$1" == "" ]]; then
    echo "You must provide an IP."
    exit 1
fi

if [[ "$2" == "" ]]; then
    echo "You must provide an SSH key."
    exit 1
fi

if [[ "$3" == "" ]]; then
    echo "You must provide a WiFi SSID."
    exit 1
fi

#if [[ "$4" == "" ]]; then
#    echo "You must provide a WiFi password."
#    exit 1
#fi

sudo chmod 600 $2

ssh -i $2 root@$1 "uname -a" > /tmp/conntest 2>> /tmp/conntest

if [[ $(cat /tmp/conntest) == *"no mutual"* ]]; then
    echo "Modifying ssh config to allow ssh-rsa"
    sudo /bin/bash -c "echo "PubkeyAcceptedKeyTypes +ssh-rsa" >>/etc/ssh/ssh_config"
    ssh -i $2 root@$1 "uname -a" > /tmp/conntest 2>> /tmp/conntest
fi

if [[ $(cat /tmp/conntest) == *"Vector"* || $(cat /tmp/conntest) == *"Cozmo"* ]]; then
    echo "Vector robot target verified"
else
    echo "Target IP does not point to a Vector or your SSH key is invalid."
    echo $(cat /tmp/conntest)
    exit 1
fi

rm /tmp/conntest

if [[ ! -f $IMAGEPATH ]]; then
    echo "You must build the robot image per the README instructions."
    exit 1
fi

#noinitrd ro console=ttyHSL0,115200,n8 androidboot.hardware=qcom ehci-hcd.park=3 msm_rtb.filter=0x37 lpm_levels.sleep_disabled=1 rootwait androidboot.bootdevice=7824900.sdhci mem=511M androidboot.bootdevice=7824900.sdhci anki.dev androidboot.baseband=apq root=/dev/mmcblk0p25 androidboot.slot_suffix=_b rootwait ro

VCMDLINE=$(ssh -i $2 root@$1 "cat /proc/cmdline")

if [[ $VCMDLINE == *'slot_suffix=_b'* ]]; then
    export ORIGSLOT=b
    export TARGETSLOT=a
elif [[ $VCMDLINE == *'slot_suffix=_a'* ]]; then
    export ORIGSLOT=a
    export TARGETSLOT=b
elif [[ $VCMDLINE == *'slot_suffix=_f'* ]]; then
    export ORIGSLOT=a
    export RECOVERY=true
    export TARGETSLOT=b
fi

if [[ $SKIPWPA != "1" ]]; then

mkdir -p edits
mount $IMAGEPATH edits

echo "Placing wpa_supplicant entry for WiFi connectivity"
echo "" >> edits/data/misc/wifi/wpa_supplicant.conf
echo "network={" >> edits/data/misc/wifi/wpa_supplicant.conf
echo "  scan_ssid=1" >> edits/data/misc/wifi/wpa_supplicant.conf
echo "  ssid=\"$3\"" >> edits/data/misc/wifi/wpa_supplicant.conf
echo "  psk=\"$4\"" >> edits/data/misc/wifi/wpa_supplicant.conf
echo "}" >> edits/data/misc/wifi/wpa_supplicant.conf

cat edits/data/misc/wifi/wpa_supplicant.conf

umount edits

fi

DDBOOTCOMMAND="dd if=/dev/block/bootdevice/by-name/boot_${ORIGSLOT} of=/dev/block/bootdevice/by-name/boot_${TARGETSLOT}"
echo $DDBOOTCOMMAND

echo "Stopping anki-robot.target (makes this faster)"
ssh -i $2 root@$1 "systemctl stop anki-robot.target"

echo "Flashing boot partition"
ssh -i $2 root@$1 ${DDBOOTCOMMAND}

echo "Flashing image to slot $TARGETSLOT (will get done at around 450MB)"
dd if=$IMAGEPATH status=progress | ssh -i $2 root@$1 "dd of=/dev/block/bootdevice/by-name/system_$TARGETSLOT"


echo "Dealing with kernel modules"
ssh -i $2 root@$1 "mount /dev/block/bootdevice/by-name/system_$TARGETSLOT /mnt"
ssh -i $2 root@$1 "rm -rf /mnt/lib/modules"
if [[ $RECOVERY == "true" ]]; then
	ssh -i $2 root@$1 "mount -o rw,remount / && mkdir -p /mnt2 && mount /dev/block/bootdevice/by-name/system_a /mnt2 && cp -r /mnt2/usr/lib/modules /mnt/lib/ && sync && umount /mnt2"
else
	ssh -i $2 root@$1 "cp -r /usr/lib/modules /mnt/lib/"
fi
ssh -i $2 root@$1 "sync && umount /mnt"
if [[ ${RECOVERY} == "true" ]]; then
	ORIGSLOT=f
fi
ssh -i $2 root@$1 "bootctl $ORIGSLOT set_active $TARGETSLOT"

echo "Rebooting Vector to new system image"
ssh -i $2 root@$1 "reboot &"


