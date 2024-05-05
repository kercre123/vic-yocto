inherit core-image

# Only when verity feature is enabled, start including related tasks.
VERITY_PROVIDER ?= "${@bb.utils.contains('DISTRO_FEATURES', 'dm-verity', 'dm-verity', '', d)}"
inherit ${VERITY_PROVIDER}

# Default deploy path for emmc images.
DEPLOY_DIR_IMAGE_EMMC ?= "${DEPLOY_DIR_IMAGE}"

# Default deploy path for nand images.
DEPLOY_DIR_IMAGE_NAND ?= "${DEPLOY_DIR_IMAGE}"
# generate a companion debug archive containing symbols from the -dbg packages  11
IMAGE_GEN_DEBUGFS = "1"
IMAGE_FSTYPES_DEBUGFS = "tar.bz2"

# Default deploy path for squashfs images.
DEPLOY_DIR_IMAGE_SQUASHFS ?= "${DEPLOY_DIR_IMAGE}"

#  Function to get most suitable .inc file with list of packages
#  to be installed into root filesystem from layer it is called.
#  Following is the order of priority.
#  P1: <basemachine>/<basemachine>-<distro>-<layerkey>-image.inc
#  P2: <basemachine>/<basemachine>-<layerkey>-image.inc
#  P3: common/common-<layerkey>-image.inc
def get_bblayer_img_inc(layerkey, d):
    distro      = d.getVar('DISTRO', True)
    basemachine = d.getVar('BASEMACHINE', True)

    lkey = ''
    if layerkey != '':
        lkey = layerkey + "-"

    common_inc  = "common-"+ lkey + "image.inc"
    machine_inc = basemachine + "-" + lkey + "image.inc"
    distro_inc  = machine_inc
    if distro == 'nad':
        distro = 'auto'
    if distro != 'base' or '':
        distro_inc = basemachine + "-" + distro +"-" + lkey + "image.inc"

    distro_inc_path  = os.path.join(d.getVar('THISDIR'), basemachine, distro_inc)
    machine_inc_path = os.path.join(d.getVar('THISDIR'), basemachine, machine_inc)
    common_inc_path  = os.path.join(d.getVar('THISDIR'), "common", common_inc)

    if os.path.exists(distro_inc_path):
        img_inc_path = distro_inc_path
    elif os.path.exists(machine_inc_path):
        img_inc_path = machine_inc_path
    else:
        img_inc_path = common_inc_path
    bb.note(" Incuding packages from %s" % (img_inc_path))
    return img_inc_path

IMAGE_INSTALL_ATTEMPTONLY ?= ""
IMAGE_INSTALL_ATTEMPTONLY[type] = "list"

RAMDISK ?= "/dev/null"
RAMDISK_OFFSET ?= "0x0"
INITRAMFS ?= "${DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE}-${MACHINE}.cpio.gz"

def get_initramfs_path(d):
    if os.path.exists(d.getVar('INITRAMFS')) and d.getVar('EXT_INITRAMFS_IMAGE_BUNDLE', True):
        return '%s' %(d.getVar('INITRAMFS'))
    return '/dev/null'

INITRAMFS_PATH = "${@get_initramfs_path(d)}"

# Original definition is in image.bbclass. Overloading it with internal list of packages
# to ensure dependencies are not messed up in case package is absent.
PACKAGE_INSTALL_ATTEMPTONLY = "${IMAGE_INSTALL_ATTEMPTONLY} ${FEATURE_INSTALL_OPTIONAL}"

# Check and remove empty packages before rootfs creation
do_rootfs[prefuncs] += "rootfs_ignore_packages"
python rootfs_ignore_packages() {
    excl_pkgs = d.getVar("PACKAGE_EXCLUDE", True).split()
    atmt_only_pkgs = d.getVar("PACKAGE_INSTALL_ATTEMPTONLY", True).split()
    inst_atmt_pkgs = d.getVar("IMAGE_INSTALL_ATTEMPTONLY", True).split()

    empty_pkgs = "${TMPDIR}/prebuilt/${MACHINE}/empty_pkgs"
    if (os.path.isfile(empty_pkgs)):
        with open(empty_pkgs) as file:
            ignore_pkgs = file.read().splitlines()
    else:
        ignore_pkgs=""

    for pkg in inst_atmt_pkgs:
        if pkg in ignore_pkgs:
            excl_pkgs.append(pkg)
            atmt_only_pkgs.remove(pkg)
            bb.debug(1, "Adding empty package %s, in %s IMAGE_INSTALL_ATTEMPTONLY to exclude list. (%s) " % (pkg, d.getVar('PN', True), excl_pkgs))

    d.setVar("PACKAGE_EXCLUDE", ' '.join(excl_pkgs))
    d.setVar("PACKAGE_INSTALL_ATTEMPTONLY", ' '.join(atmt_only_pkgs))
}

# Call function makesystem to generate sparse ext4 image
python __anonymous () {
    machine = d.getVar("MACHINE", True)
    if bb.utils.contains('IMAGE_FSTYPES', 'ext4', True, False, d):
        bb.build.addtask('makesystem', 'do_image_qa', 'do_rootfs', d)

# Call function image_squashfs_xz before executing do_make_ramdisk_bootimg
    if bb.utils.contains('DISTRO_FEATURES', 'flashless', True, False, d):
        if bb.utils.contains('IMAGE_FSTYPES', 'squashfs-xz', True, False, d):
            bb.build.addtask('do_make_ramdisk_bootimg', 'do_image_complete', 'do_image_squashfs_xz', d)
}

### Generate system.img #####
# Alter system image size if varity is enabled.
do_makesystem[prefuncs]  += " ${@bb.utils.contains('DISTRO_FEATURES', 'dm-verity', 'adjust_system_size_for_verity', '', d)}"
do_makesystem[postfuncs] += " ${@bb.utils.contains('DISTRO_FEATURES', 'dm-verity', 'make_verity_enabled_system_image', '', d)}"
do_makesystem[dirs]       = "${DEPLOY_DIR_IMAGE}"

################################################
############# Generate boot.img ################
################################################
python do_make_bootimg () {
    import subprocess

#create emmc deploy dir.
    emmc_deploy = d.getVar('DEPLOY_DIR_IMAGE_EMMC', True)
    if not os.path.exists(emmc_deploy):
     os.mkdir(emmc_deploy)

#create nand deploy dir.
    nand_deploy = d.getVar('DEPLOY_DIR_IMAGE_NAND', True)
    if not os.path.exists(nand_deploy):
     os.mkdir(nand_deploy)

    xtra_parms=""
    mkboot_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/mkbootimg'
    bundle_initramfs = d.getVar('INITRAMFS_IMAGE_BUNDLE', True)
    bundle_ext_initramfs = d.getVar('EXT_INITRAMFS_IMAGE_BUNDLE', True)
    if bundle_initramfs == '1' and bundle_ext_initramfs == '1':
         bb.fatal("bundle_initramfs and bundle_ext_initramfs are mutually exclusive features")

    initramfs_path = d.getVar('INITRAMFS_PATH', True)

    if bundle_initramfs == '1':
        zimg_path       = d.getVar('DEPLOY_DIR_IMAGE', True) + "/" + d.getVar('KERNEL_IMAGETYPE', True) + ".initramfs"
    else:
        zimg_path       = d.getVar('DEPLOY_DIR_IMAGE', True) + "/" + d.getVar('KERNEL_IMAGETYPE', True)
    cmdline         = "\"" + d.getVar('KERNEL_CMD_PARAMS', True) + "\""
    pagesize        = d.getVar('PAGE_SIZE', True)
    base            = d.getVar('KERNEL_BASE', True)

    output_emmc          = d.getVar('DEPLOY_DIR_IMAGE_EMMC', True) + "/" + d.getVar('BOOTIMAGE_TARGET', True)
    output_nand          = d.getVar('DEPLOY_DIR_IMAGE_NAND', True) + "/" + d.getVar('BOOTIMAGE_TARGET', True)

    # When verity is enabled add '.noverity' suffix to default boot img.
    if bb.utils.contains('DISTRO_FEATURES', 'dm-verity', True, False, d):
            output_emmc += ".noverity"

    # cmd to make boot.img for ext4
    if bb.utils.contains('IMAGE_FSTYPES', 'ext4', True, False, d):
         cmd_emmc =  mkboot_bin_path + " --kernel %s --cmdline %s --pagesize %s --base %s %s --ramdisk /dev/null --ramdisk_offset 0x0 --output %s" \
           % (zimg_path, cmdline, pagesize, base, xtra_parms, output_emmc )
         bb.debug(1, "do_make_bootimg cmd_emmc: %s" % (cmd_emmc))
         subprocess.call(cmd_emmc, shell=True)

    if bb.utils.contains('IMAGE_FSTYPES', 'ubi', True, False, d):
         xtra_parms = " --tags-addr" + " " + d.getVar('KERNEL_TAGS_OFFSET')
         cmd_nand =  mkboot_bin_path + " --kernel %s --cmdline %s --pagesize %s --base %s --ramdisk %s --ramdisk_offset 0x0 %s --output %s" \
                   % (zimg_path, cmdline, pagesize, base, initramfs_path, xtra_parms, output_nand )
         bb.debug(1, "do_make_bootimg cmd_nand: %s" % (cmd_nand))
         subprocess.call(cmd_nand, shell=True)
}
do_make_bootimg[dirs]      = "${DEPLOY_DIR_IMAGE}"
# Make sure native tools and vmlinux ready to create boot.img
do_make_bootimg[depends]  += "${PN}:do_prepare_recipe_sysroot"
do_make_bootimg[depends]  += "virtual/kernel:do_shared_workdir"
do_make_bootimg[depends]  += " ${@bb.utils.contains('INITRAMFS_IMAGE_BUNDLE', '1', 'linux-msm:do_bundle_initramfs', '', d)}"
do_make_bootimg[depends]  += " ${@bb.utils.contains('EXT_INITRAMFS_IMAGE_BUNDLE', '1', '${INITRAMFS_IMAGE}:do_image_complete', '', d)}"

addtask do_make_bootimg before do_image_complete

################################################
##########  Generate ramdisk boot.img ##########
################################################
python do_make_ramdisk_bootimg () {
    import subprocess

# create squashfs deploy dir.
    squashfs_deploy = d.getVar('DEPLOY_DIR_IMAGE_SQUASHFS', True)
    if not os.path.exists(squashfs_deploy):
     os.mkdir(squashfs_deploy)

    mkboot_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/mkbootimg'
    zimg_path       = d.getVar('DEPLOY_DIR_IMAGE', True) + "/" + d.getVar('KERNEL_IMAGETYPE', True)
    cmdline         = "\"" + d.getVar('KERNEL_CMD_PARAMS', True) + "\""
    pagesize        = d.getVar('PAGE_SIZE', True)
    base            = d.getVar('KERNEL_BASE', True)

    output_squashfs      = d.getVar('DEPLOY_DIR_IMAGE_SQUASHFS', True) + "/" + d.getVar('MACHINE', True) + "-flashless-boot.img"

    # Get the squashfs ramdisk
    ramdisk         = d.getVar('RAMDISK', True)
    ramdisk_offset  = d.getVar('RAMDISK_OFFSET', True)

    # cmd to make boot.img for ext4
    xtra_parms = " --tags-addr" + " " + d.getVar('KERNEL_TAGS_OFFSET')

    cmd_squashfs =  mkboot_bin_path + " --kernel %s --cmdline %s --pagesize %s --base %s %s --ramdisk %s --ramdisk_offset %s --output %s" \
      % (zimg_path, cmdline, pagesize, base, xtra_parms, ramdisk, ramdisk_offset, output_squashfs )
    bb.debug(1, "do_make_ramdisk_bootimg cmd_squashfs: %s" % (cmd_squashfs))
    subprocess.call(cmd_squashfs, shell=True)
}
do_make_ramdisk_bootimg[dirs]      = "${DEPLOY_DIR_IMAGE}"
# Make sure native tools and vmlinux ready to create ramdisk boot.img
do_make_ramdisk_bootimg[depends]  += "${PN}:do_prepare_recipe_sysroot"
do_make_ramdisk_bootimg[depends]  += "virtual/kernel:do_shared_workdir"

# With dm-verity, kernel cmdline has to be updated with correct hash value of
# system image. This means final boot image can be created only after system image.
# But many a times when only kernel need to be built waiting for full image is
# time consuming. To over come this make_veritybootimg task is added to build boot
# img with verity. Normal do_make_bootimg continue to build boot.img without verity.
python do_make_veritybootimg () {
    import subprocess

    xtra_parms=""
    verity_cmdline = ""
    if bb.utils.contains('DISTRO_FEATURES', 'dm-verity', True, False, d):
        verity_cmdline = get_verity_cmdline(d).strip()

    mkboot_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/mkbootimg'
    zimg_path       = d.getVar('DEPLOY_DIR_IMAGE', True) + "/" + d.getVar('KERNEL_IMAGETYPE', True)
    cmdline         = "\"" + d.getVar('KERNEL_CMD_PARAMS', True) + " " + verity_cmdline + "\""
    pagesize        = d.getVar('PAGE_SIZE', True)
    base            = d.getVar('KERNEL_BASE', True)
    output_emmc     = d.getVar('DEPLOY_DIR_IMAGE_EMMC', True) + "/" + d.getVar('BOOTIMAGE_TARGET', True)

    # cmd to make boot.img
    cmd =  mkboot_bin_path + " --kernel %s --cmdline %s --pagesize %s --base %s %s --ramdisk /dev/null --ramdisk_offset 0x0 --output %s" \
           % (zimg_path, cmdline, pagesize, base, xtra_parms, output_emmc )

    bb.debug(1, "do_make_veritybootimg cmd: %s" % (cmd))

    subprocess.call(cmd, shell=True)
}
do_make_veritybootimg[depends]  += "${PN}:do_makesystem"

python () {
    bundle_initramfs = d.getVar('INITRAMFS_IMAGE_BUNDLE', True)
    bundle_ext_initramfs = d.getVar('EXT_INITRAMFS_IMAGE_BUNDLE', True)
    if bundle_initramfs != '1' and bundle_ext_initramfs != '1':
        if bb.utils.contains('DISTRO_FEATURES', 'dm-verity', True, False, d):
            bb.build.addtask('do_make_veritybootimg', 'do_image_complete', 'do_rootfs', d)
}
