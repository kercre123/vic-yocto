DEPENDS += "${@["","machine-recovery-image releasetools-native"][(d.getVar('RECOVERY_IMAGE', True) == '1')]}"

# Add tasks to generate recovery image, OTA zip files
python __anonymous () {
    if d.getVar('RECOVERY_IMAGE', True) == '1' and bb.utils.contains('IMAGE_FSTYPES', 'ubi', True, False, d):
        bb.build.addtask('do_recovery_ubi', 'do_build', 'do_rootfs', d)
        bb.build.addtask('do_gen_otazip_ubi', 'do_build', 'do_recovery_ubi', d)
}

OTA_TARGET_IMAGE_ROOTFS_UBI = "${IMAGE_ROOTFS}/../${MACHINE}-ota-target-image-ubi"
RECOVERY_IMAGE_ROOTFS        = "$(echo ${IMAGE_ROOTFS} | sed 's#${PN}#machine-recovery-image#')"

#Create directory structure for targetfiles.zip
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/BOOTABLE_IMAGES"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/DATA"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/META"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/OTA"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/SYSTEM"
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/RADIO"

# Create this folder just for saving file_contexts(SElinux security context file),
# It will be used to generate OTA packages when selinux_fc is set.
do_recovery_ubi[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_UBI}/BOOT/RAMDISK"

# recovery rootfs is required for generating OTA files.
# Wait till all tasks of machine-recovery-image complete.
do_recovery_ubi[depends] += "machine-recovery-image:do_build"

do_recovery_ubi() {
    echo "base image rootfs: ${IMAGE_ROOTFS}"
    echo "recovery image rootfs: ${RECOVERY_IMAGE_ROOTFS}"

    # copy radio directory content into RADIO directory
    cp -r ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/${BASEMACHINE}/radio/. ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RADIO/.

    # copy the boot\recovery images
    cp ${DEPLOY_DIR_IMAGE}/${MACHINE}-boot.img ${OTA_TARGET_IMAGE_ROOTFS_UBI}/BOOTABLE_IMAGES/boot.img

    cp ${DEPLOY_DIR_IMAGE}/${MACHINE}-boot.img ${OTA_TARGET_IMAGE_ROOTFS_UBI}/BOOTABLE_IMAGES/recovery.img

    # copy the contents of system rootfs
    cp -r ${IMAGE_ROOTFS}/. ${OTA_TARGET_IMAGE_ROOTFS_UBI}/SYSTEM/.
    cd  ${OTA_TARGET_IMAGE_ROOTFS_UBI}/SYSTEM
    rm -rf var/run
    ln -snf ../run var/run

    #copy contents of recovery rootfs
    cp -r ${RECOVERY_IMAGE_ROOTFS}/. ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/.

    #generate recovery.fstab which is used by the updater-script
    #echo #mount point fstype device [device2] >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/recovery.fstab
    echo /boot     mtd     boot >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/recovery.fstab
    echo /cache    ubifs  cache >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/recovery.fstab
    echo /data     ubifs  userdata >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/recovery.fstab
    echo /recovery mtd     recovery >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/recovery.fstab
    echo /system   ubifs  system >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/recovery.fstab

    #Copy contents of userdata rootfs
    if [ -d ${TMPDIR}/rootfs/${MACHINE}-usrfs/ ]; then
       cp -r ${TMPDIR}/rootfs/${MACHINE}-usrfs/. ${OTA_TARGET_IMAGE_ROOTFS_UBI}/DATA/.
    fi

    #Getting content for OTA folder
    mkdir -p ${OTA_TARGET_IMAGE_ROOTFS_UBI}/OTA/bin
    cp   ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/usr/bin/applypatch ${OTA_TARGET_IMAGE_ROOTFS_UBI}/OTA/bin/.

    cp   ${OTA_TARGET_IMAGE_ROOTFS_UBI}/RECOVERY/usr/bin/updater ${OTA_TARGET_IMAGE_ROOTFS_UBI}/OTA/bin/.

    #cp and modify file_contexts to BOOT/RAMDISK folder
    if [[ "${DISTRO_FEATURES}" =~ "selinux" ]]; then
        cp ${IMAGE_ROOTFS}/etc/selinux/mls/contexts/files/file_contexts ${OTA_TARGET_IMAGE_ROOTFS_UBI}/BOOT/RAMDISK/.
        sed -i 's#^/#/system/#g' ${OTA_TARGET_IMAGE_ROOTFS_UBI}/BOOT/RAMDISK/file_contexts
        #set selinux_fc
        echo selinux_fc=BOOT/RAMDISK/file_contexts >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt
        #set use_set_metadata to 1 so that updater-script
        #will have rules to apply selabels
        echo use_set_metadata=1 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt
    fi

    # copy contents of META folder
    #recovery_api_version is from recovery module
    echo recovery_api_version=3 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #blocksize = BOARD_FLASH_BLOCK_SIZE
    echo blocksize=131072 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #boot_size: Size of boot partition from partition.xml
    echo boot_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #recovery_size : Size of recovery partition from partition.xml
    echo recovery_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #system_size : Size of system partition from partition.xml
    echo system_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #userdate_size : Size of data partition from partition.xml
    echo userdata_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #cache_size : Size of data partition from partition.xml
    echo cache_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #mkyaffs2_extra_flags : -c $(BOARD_KERNEL_PAGESIZE) -s $(BOARD_KERNEL_SPARESIZE)
    echo mkyaffs2_extra_flags=-c 4096 -s 16 >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #extfs_sparse_flag : definition in build
    echo extfs_sparse_flags=-s >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt

    #default_system_dev_certificate : Dummy location
    echo default_system_dev_certificate=build/abcd >> ${OTA_TARGET_IMAGE_ROOTFS_UBI}/META/misc_info.txt
}

# Task to generate OTA zip files
python do_gen_otazip_ubi() {
    import subprocess

    cmd1 = "cd ${IMAGE_ROOTFS}/../${MACHINE}-ota-target-image-ubi && zip -qry ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools/target-files-ubi.zip *"
    subprocess.call(cmd1, shell=True)

    cmd2 = "cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && ./full_ota.sh target-files-ubi.zip ${IMAGE_ROOTFS} ubi"
    subprocess.call(cmd2, shell=True)

    cmd3 = "cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && cp update_ubi.zip target-files-ubi.zip ${DEPLOY_DIR_IMAGE}"
    subprocess.call(cmd3, shell=True)
}
