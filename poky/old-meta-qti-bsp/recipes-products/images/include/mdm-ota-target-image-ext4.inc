DEPENDS += "${@["","machine-recovery-image releasetools-native"][(d.getVar('RECOVERY_IMAGE', True) == '1')]}"

# Add tasks to generate recovery image, OTA zip files
python __anonymous () {
    if d.getVar('RECOVERY_IMAGE', True) == '1' and bb.utils.contains('IMAGE_FSTYPES', 'ext4', True, False, d):
        bb.build.addtask('do_recovery_ext4', 'do_build', 'do_rootfs', d)
        bb.build.addtask('do_gen_otazip_ext4', 'do_build', 'do_recovery_ext4', d)
}

OTA_TARGET_IMAGE_ROOTFS_EXT4 = "${IMAGE_ROOTFS}/../${MACHINE}-ota-target-image-ext4"
RECOVERY_IMAGE_ROOTFS        = "$(echo ${IMAGE_ROOTFS} | sed 's#${PN}#machine-recovery-image#')"

#Create directory structure for targetfiles.zip
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOTABLE_IMAGES"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/DATA"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/SYSTEM"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RADIO"

# Create this folder just for saving file_contexts(SElinux security context file),
# It will be used to generate OTA packages when selinux_fc is set.
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOT/RAMDISK"

# recovery rootfs is required for generating OTA files.
# Wait till all tasks of machine-recovery-image complete.
do_recovery_ext4[depends] += "machine-recovery-image:do_build"

do_recovery_ext4() {
    echo "base image rootfs: ${IMAGE_ROOTFS}"
    echo "recovery image rootfs: ${RECOVERY_IMAGE_ROOTFS}"

    # copy radio directory content into RADIO directory
    cp -r ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/${BASEMACHINE}/radio/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RADIO/.

    # copy the boot\recovery images
    cp ${DEPLOY_DIR_IMAGE}/${MACHINE}-boot.img ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOTABLE_IMAGES/boot.img

    cp ${DEPLOY_DIR_IMAGE}/${MACHINE}-boot.img ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOTABLE_IMAGES/recovery.img

    # copy the contents of system rootfs
    cp -r ${IMAGE_ROOTFS}/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/SYSTEM/.
    cd  ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/SYSTEM
    rm -rf var/run
    ln -snf ../run var/run

    #copy contents of recovery rootfs
    cp -r ${RECOVERY_IMAGE_ROOTFS}/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/.

    #generate recovery.fstab which is used by the updater-script
    #echo #mount point fstype device [device2] >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab
    echo /boot     emmc  /dev/block/bootdevice/by-name/boot >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab
    echo /cache    ext4  cache >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab
    echo /data     ext4  userdata >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab
    echo /recovery emmc  /dev/block/bootdevice/by-name/recovery >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab
    echo /system   ext4  system >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab

    #Copy contents of userdata rootfs
    if [ -d ${TMPDIR}/rootfs/${MACHINE}-usrfs/ ]; then
       cp -r ${TMPDIR}/rootfs/${MACHINE}-usrfs/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/DATA/.
    fi

    #Getting content for OTA folder
    mkdir -p ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA/bin
    cp   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/usr/bin/applypatch ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA/bin/.

    cp   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/usr/bin/updater ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA/bin/.

    #cp and modify file_contexts to BOOT/RAMDISK folder
    if [[ "${DISTRO_FEATURES}" =~ "selinux" ]]; then
        cp ${IMAGE_ROOTFS}/etc/selinux/mls/contexts/files/file_contexts ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOT/RAMDISK/.
        sed -i 's#^/#/system/#g' ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOT/RAMDISK/file_contexts
        #set selinux_fc
        echo selinux_fc=BOOT/RAMDISK/file_contexts >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
        #set use_set_metadata to 1 so that updater-script
        #will have rules to apply selabels
        echo use_set_metadata=1 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
    fi

    # copy contents of META folder
    #recovery_api_version is from recovery module
    echo recovery_api_version=3 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    #blocksize = BOARD_FLASH_BLOCK_SIZE
    echo blocksize=131072 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    if [ "${BASEMACHINE}" == "apq8053" ] || [ "${BASEMACHINE}" == "apq8009" ] || [ "${BASEMACHINE}" == "apq8017" ]; then
       echo boot_size=0x01E00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

       #recovery_size : Size of recovery partition from partition.xml
       echo recovery_size=0x01E00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
    elif [ "${BASEMACHINE}" == "apq8096" ]; then
       #boot_size: Size of boot partition from partition.xml
       echo boot_size=0x04000000  >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

       #recovery_size : Size of recovery partition from partition.xml
       echo recovery_size=0x04000000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
    else
       #boot_size: Size of boot partition from partition.xml
       echo boot_size=0x00F00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

       #recovery_size : Size of recovery partition from partition.xml
       echo recovery_size=0x00F00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
    fi

    #system_size : Size of system partition from partition.xml
    echo system_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    #userdate_size : Size of data partition from partition.xml
    echo userdata_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    #cache_size : Size of data partition from partition.xml
    echo cache_size=0x00A00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    #mkyaffs2_extra_flags : -c $(BOARD_KERNEL_PAGESIZE) -s $(BOARD_KERNEL_SPARESIZE)
    echo mkyaffs2_extra_flags=-c 4096 -s 16 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    #extfs_sparse_flag : definition in build
    echo extfs_sparse_flags=-s >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    #default_system_dev_certificate : Dummy location
    echo default_system_dev_certificate=build/abcd >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
}

# Task to generate OTA zip files
python do_gen_otazip_ext4() {
    import subprocess

    cmd1 = "cd ${IMAGE_ROOTFS}/../${MACHINE}-ota-target-image-ext4 && zip -qry ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools/target-files-ext4.zip *"
    subprocess.call(cmd1, shell=True)

    cmd2 = "cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && ./full_ota.sh target-files-ext4.zip ${IMAGE_ROOTFS} ext4"
    subprocess.call(cmd2, shell=True)

    cmd3 = "cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && cp update_ext4.zip target-files-ext4.zip ${DEPLOY_DIR_IMAGE}"
    subprocess.call(cmd3, shell=True)
}

