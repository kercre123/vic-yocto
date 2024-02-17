# To add OTA upgrade support on an emmc/ufs target,
# add the MACHINE name to this list.
# This is the "only" list that will control whether
# OTA upgrade will be supported on a target.
OTA_SUPPORTED_EMMC_TARGET_LIST = "apq8009 qcs405-som1 qcs605-64 qcs605-32 sdm845 sa2150p sdmsteppe nf-32 nf-64 vt-64"

#deploy dir for emmc images.
DEPLOY_DIR_IMAGE_EMMC ?= "${DEPLOY_DIR_IMAGE}"

#default emmc rootfs dir.
EMMC_ROOTFS_DIR ?= "${IMAGE_ROOTFS}"
def emmc_target_supports_OTA_upgrade(d):
    list_of_supported_targets = d.getVar('OTA_SUPPORTED_EMMC_TARGET_LIST');

    if d.getVar('MACHINE', True) in list_of_supported_targets:
      return True

    return False

def emmc_set_vars_and_get_dependencies(d):
    if not emmc_target_supports_OTA_upgrade(d):
        d.setVar('RECOVERY_IMAGE', "0");
        d.setVar('GENERATE_AB_OTA_PACKAGE', "0");
        # Do not create machine-recovery-image or the OTA packages
        return ""

    if bb.utils.contains('DISTRO_FEATURES', 'ab-boot-support', True, False, d):
        # if A/B support is supported, recovery image need not be generated.
        # only A/B target will be generated
        d.setVar('RECOVERY_IMAGE', "0");
        d.setVar('GENERATE_AB_OTA_PACKAGE', "1");
        return " releasetools-native"
    else:
        # for Non A/B target, set RECOVERY_IMAGE to "1"
        # this will generate a non A/B update package as well.
        d.setVar('RECOVERY_IMAGE', "1");
        d.setVar('GENERATE_AB_OTA_PACKAGE', "0");
        return " machine-recovery-image releasetools-native"

# Add tasks to generate recovery image, OTA zip files
python __anonymous () {
    if bb.utils.contains('IMAGE_FSTYPES', 'ext4', True, False, d):
        d.appendVar('DEPENDS', emmc_set_vars_and_get_dependencies(d));
        if (d.getVar('RECOVERY_IMAGE', True) == '1' or
                d.getVar('GENERATE_AB_OTA_PACKAGE', True) == '1'):
            if (d.getVar('DEBUG_BUILD', True) != '1'):
                bb.build.addtask('do_recovery_ext4', 'do_build', 'do_image_complete', d)
                bb.build.addtask('do_gen_otazip_ext4', 'do_build', 'do_recovery_ext4', d)
}

OTA_TARGET_IMAGE_ROOTFS_EXT4 = "${IMAGE_ROOTFS}/../${MACHINE}-ota-target-image-ext4"
NON_AB_RECOVERY_IMAGE_ROOTFS = "$(echo ${IMAGE_ROOTFS} | sed 's#${PN}#machine-recovery-image#')"

# If A/B package is to be generated, recoveryfs's rootfs is same as system's rootfs
RECOVERY_IMAGE_ROOTFS = "${@["${NON_AB_RECOVERY_IMAGE_ROOTFS}", "${EMMC_ROOTFS_DIR}"][(d.getVar('GENERATE_AB_OTA_PACKAGE', True) == '1')]}"

#Create directory structure for targetfiles.zip
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOTABLE_IMAGES"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/DATA"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/SYSTEM"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RADIO"
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/IMAGES"

# Create this folder just for saving file_contexts(SElinux security context file),
# It will be used to generate OTA packages when selinux_fc is set.
do_recovery_ext4[cleandirs] += "${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOT/RAMDISK"

# recovery rootfs is required for generating OTA files.
# Wait till all tasks of machine-recovery-image complete.
do_recovery_ext4[depends] += "${@ 'machine-recovery-image:do_build' if d.getVar('RECOVERY_IMAGE') == '1' else ''}"
do_recovery_ext4[depends] += "machine-image:do_make_bootimg"

do_recovery_ext4() {
    echo "base image rootfs: ${IMAGE_ROOTFS}"
    echo "recovery image rootfs: ${RECOVERY_IMAGE_ROOTFS}"

    # copy radio directory content into RADIO directory
    cp -r ${WORKSPACE}/poky/meta-qti-bsp/recipes-bsp/base-files-recovery/${MACHINE}/radio/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RADIO/.

    # copy the boot\recovery images
    cp ${DEPLOY_DIR_IMAGE_EMMC}/${BOOTIMAGE_TARGET} ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOTABLE_IMAGES/boot.img

    cp ${DEPLOY_DIR_IMAGE_EMMC}/${BOOTIMAGE_TARGET}   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOTABLE_IMAGES/recovery.img
    cp ${DEPLOY_DIR_IMAGE_EMMC}/${BOOTIMAGE_TARGET}   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/IMAGES/recovery.img
    cp ${DEPLOY_DIR_IMAGE_EMMC}/${BOOTIMAGE_TARGET}   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/IMAGES/boot.img
    cp ${DEPLOY_DIR_IMAGE_EMMC}/${SYSTEMIMAGE_TARGET} ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/IMAGES/system.img
    if [ -e ${DEPLOY_DIR_IMAGE}/system.map ]; then
       cp ${DEPLOY_DIR_IMAGE}/system.map ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/IMAGES/system.map
    fi

    # copy the contents of system rootfs
    cp -r ${EMMC_ROOTFS_DIR}/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/SYSTEM/.
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
    echo /system   ext4  /dev/block/bootdevice/by-name/system >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/recovery.fstab

    #Copy contents of userdata rootfs
    if [ -d ${TMPDIR}/rootfs/${MACHINE}-usrfs/ ]; then
       cp -r ${TMPDIR}/rootfs/${MACHINE}-usrfs/. ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/DATA/.
    fi

    #Getting content for OTA folder
    mkdir -p ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA/bin
    cp   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/usr/bin/applypatch ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA/bin/.

    cp   ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/RECOVERY/usr/bin/updater ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/OTA/bin/.

    # Pack releasetools.py into META folder itself.
    # This could also have been done by passing "--device_specific" to
    # ota_from_target_files.py but it would be hacky to find the absolute path there.
    cp ${WORKSPACE}/OTA/device/qcom/common/releasetools.py ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/.

    #cp and modify file_contexts to BOOT/RAMDISK folder
    if [[ "${DISTRO_FEATURES}" =~ "selinux" ]]; then
        cp ${EMMC_ROOTFS_DIR}/etc/selinux/mls/contexts/files/file_contexts ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/BOOT/RAMDISK/.
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

    if [ "${BASEMACHINE}" == "apq8053" ] || [ "${BASEMACHINE}" == "apq8009" ]; then
       echo boot_size=0x01E00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

       #recovery_size : Size of recovery partition from partition.xml
       echo recovery_size=0x01E00000 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
    elif [ "${BASEMACHINE}" == "apq8096" ] || [ "${BASEMACHINE}" == "apq8017" ] || \
         [ "${BASEMACHINE}" == "qcs605" ] || [ "${BASEMACHINE}" == "sdm845" ] || \
         [ "${BASEMACHINE}" == "sdmsteppe" ]; then
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

    # set block img diff version to v3
    echo "blockimgdiff_versions=3" >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt

    # Targets that support A/B boot do not need recovery(fs)-updater
    if ${@bb.utils.contains('DISTRO_FEATURES','ab-boot-support','true','false',d)}; then
        echo le_target_supports_ab=1 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
    else
        if [ "${BASEMACHINE}" == "apq8053" ] || [ "${BASEMACHINE}" == "apq8009" ]; then
            echo recovery_upgrade_supported=1 >> ${OTA_TARGET_IMAGE_ROOTFS_EXT4}/META/misc_info.txt
        fi
    fi

}

# Generate OTA zip files
do_gen_otazip_ext4() {

    # Clean up any existing target-files*.zip as this can lead to incorrect content getting packed in the zip.
    rm -f ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools/target-files-ext4.zip

    cd ${IMAGE_ROOTFS}/../${MACHINE}-ota-target-image-ext4 && /usr/bin/zip -qry ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools/target-files-ext4.zip *

    # Pack recoveryfs images only for non A/B OTA
    if [ "${RECOVERY_IMAGE}" == "1" ]; then
        cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && zip -qry target-files-ext4.zip BOOTABLE_IMAGES/recovery-unsparsed.ext4
    fi

    # Since ext4 filesystems are being used only on emmc/ufs devices,
    # use block-based OTA by default.
    block_based="--block"

    cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && ./full_ota.sh target-files-ext4.zip ${IMAGE_ROOTFS}/../emmc-rootfs  ext4 $block_based

    cd ${TMPDIR}/work/x86_64-linux/releasetools-native/1.0-r0/releasetools && cp update_ext4.zip target-files-ext4.zip ${DEPLOY_DIR_IMAGE_EMMC}
}
