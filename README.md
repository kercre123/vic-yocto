# vic-yocto

Vector's original OS is built with an old version of Yocto/OpenEmbedded. This project aims to upgrade that.

## Vector's Original OS Info

-	Kernel: 3.18.66
-	glibc: 2.22
-	arch: armel
-	yocto: jethro (2.0.3)

## Status

-	Kernel: 3.18.66
-	glibc: 2.28
-	arch: armhf
-	yocto: thud (2.6)

## Notes

-	It would be nice to get to version 3.1 (dunfell) as that is still being supported, but a LOT would need to be modified in order for that to work.
-	I have attempted the msm-4.9 kernel, but haven't been able to get it to boot at all. The code is still included at ./kernel/msm-4.9.
-	All of the meta-qti-* stuff is open-source, but there is no publicly available documentation for which branches go with which yocto version.
	-	The branches I have chosen are in ./BRANCHES.txt
-	WLAN is implemented and will connect on bootup. /data/misc/wifi/wpa_supplicant.conf must be modified to include your WiFi credentials (install.sh does automatically).
-	A test program will launch at startup. Use one of the wheels and the button to navigate it.

## Build

### From scratch

Make sure you have Docker installed.

```
git clone https://github.com/kercre123/vic-yocto
cd vic-yocto/poky
./docker.sh
chmod 0777 .
chown -R builduser ./*
su builduser
cd opensource/poky
source build/conf/set_bb_env.sh
build-victor-robot-image
```

### Update then build

If you have built this before and want to build new code:

```
cd vic-yocto/poky
./docker.sh
chmod 0777 .
chown -R builduser ./*
su builduser
cd opensource
git pull
cd poky
rm -rf build/tmp-glibc
rm -rf build/cache
rm -rf build/sstate-cache
source build/conf/set_bb_env.sh
build-victor-robot-image
```

Result will be in poky/build/tmp-glibc/deploy/images/

## Install

An install script is included. This patches the image to include the kernel modules corresponding to the boot partition of the current slot in your bot, and adds WiFi credentials for automatic connection.

```
# if you are in the docker container, run `exit` twice to get back to the host shell
cd ../install-image
# replace vectorip with vector's ip address, /path/to/sshkey with the path to his ssh key, ssid with your network name, password with your network password
sudo ./install.sh vectorip /path/to/sshkey "ssid" "password"
```

He should eventually boot up to a screen showing "booted!" and SSH should be available (with [this key](http://wire.my.to:81/ssh_root_key)).

## Resources

-	Qualcomm LE (linux embedded) repo: https://git.codelinaro.org/clo/le
-	Migration guide for thud: https://docs.yoctoproject.org/migration-guides/migration-2.6.html
-	Migration guide for sumo (includes new function definitions): https://docs.yoctoproject.org/migration-guides/migration-2.5.html
-	Original Vector tarball: https://web.archive.org/web/20221102004123if_/https://anki-vic-pubfiles.anki.com/license/prod/1.0.0/licences/OStarball.v160.tgz
-	Original Vector tarball (fixed so it builds): https://keriganc.com/opensource.tar.gz
