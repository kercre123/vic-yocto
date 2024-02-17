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
-	Currently, the built image boots. You need to hook up UART to get to a shell.
-	WLAN is functional, so you could get SSH working.
-	USB probably works too, though I haven't tested it.

## Build

Make sure you have Docker installed.

```
git clone https://github.com/kercre123/vic-yocto
cd vic-yocto/poky
./docker.sh
chmod 0777 .
su builduser
cd opensource/poky
source build/conf/set_bb_env.sh
build-victor-robot-image
```

Result will be in poky/build/tmp-glibc/deploy/images/

## Resources

-	Qualcomm LE (linux embedded) repo: https://git.codelinaro.org/clo/le
-	Migration guide for thud: https://docs.yoctoproject.org/migration-guides/migration-2.6.html
-	Migration guide for sumo (includes new function definitions): https://docs.yoctoproject.org/migration-guides/migration-2.5.html
-	Original Vector tarball: https://web.archive.org/web/20221102004123if_/https://anki-vic-pubfiles.anki.com/license/prod/1.0.0/licences/OStarball.v160.tgz
-	Original Vector tarball (fixed so it builds): https://keriganc.com/opensource.tar.gz
