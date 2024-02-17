# This class provides utilities to generate and append verity metadata
# into images as required by device-mapper-verity feature.

DEPENDS += " ${@bb.utils.contains('DISTRO_FEATURES', 'dm-verity', 'verity-utils-native', '', d)}"

FIXED_SALT = "aee087a5be3b982978c923f566a94613496b417f2af592639bc80d141e34dfe7"
BLOCK_SIZE = "4096"
BLOCK_DEVICE_SYSTEM = "/dev/block/bootdevice/by-name/system"
ORG_SYSTEM_SIZE_EXT4 = "0"
VERITY_SIZE = "0"
ROOT_HASH = ""
HASH_ALGO = "sha256"
MAPPER_DEVICE = "verity"
UPSTREAM_VERITY_VERSION = "1"
DATA_BLOCK_START = "0"
DM_KEY_PREFIX = '"'
DATA_BLOCKS_NUMBER ?= ""
SIZE_IN_SECTORS = ""
FEC_OFFSET = "0"
FEC_SIZE = "0"

FEC_SUPPORT = "1"
DEPENDS += " ${@bb.utils.contains('FEC_SUPPORT', '1', 'fec-native', '', d)}"

VERITY_IMAGE_DIR     ?= "${DEPLOY_DIR_IMAGE}/verity"
SPARSE_SYSTEM_IMG    ?= "${DEPLOY_DIR_IMAGE}/${SYSTEMIMAGE_TARGET}"
VERITY_IMG           = "${VERITY_IMAGE_DIR}/verity.img"
VERITY_METADATA_IMG  = "${VERITY_IMAGE_DIR}/verity-metadata.img"
VERITY_FEC_IMG       = "${VERITY_IMAGE_DIR}/verity-fec.img"
VERITY_CMDLINE       = "${VERITY_IMAGE_DIR}/cmdline"

python adjust_system_size_for_verity () {
    partition_size = int(d.getVar("SYSTEM_SIZE_EXT4",True))
    block_size = int(d.getVar("BLOCK_SIZE",True))
    fec_support = d.getVar("FEC_SUPPORT",True)
    hi = partition_size
    if hi % block_size != 0:
        hi = (hi // block_size) * block_size
    verity_size = get_verity_size(d, hi, fec_support)
    lo = partition_size - verity_size
    result = lo
    while lo < hi:
        i = ((lo + hi) // (2 * block_size)) * block_size
        v = get_verity_size(d, i, fec_support)
        if i + v <= partition_size:
            if result < i:
                result = i
                verity_size = v
            lo = i + block_size
        else:
            hi = i
    data_blocks_number = (result // block_size)
    fec_size = int(d.getVar("FEC_SIZE",True))
    size_in_sectors = data_blocks_number * 8
    if fec_size !=0:
        fec_off = (partition_size - fec_size) // block_size
    else:
        fec_off = 0

    d.setVar('SIZE_IN_SECTORS', str(size_in_sectors))
    d.setVar('DATA_BLOCKS_NUMBER', str(data_blocks_number))
    d.setVar('SYSTEM_SIZE_EXT4', str(result))
    d.setVar('VERITY_SIZE', str(verity_size))
    d.setVar('ORG_SYSTEM_SIZE_EXT4', str(partition_size))
    d.setVar('FEC_OFFSET', str(fec_off))

    bb.debug(1, "Data Blocks Number: %s" % d.getVar('DATA_BLOCKS_NUMBER', True))
    bb.debug(1, "FEC Offset: %s" % d.getVar("FEC_OFFSET",True))
    bb.debug(1, "system image size without verity: %s" % d.getVar("ORG_SYSTEM_SIZE_EXT4",True))
    bb.debug(1, "verity size: %s" % d.getVar("VERITY_SIZE",True))
    bb.debug(1, "system image size with verity: %s" % d.getVar("SYSTEM_SIZE_EXT4",True))
    bb.note("System image size is adjusted with verity")
}

def get_verity_size(d, partition_size, fec_support):
    import subprocess

    # Get verity tree size
    bvt_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/build_verity_tree'
    cmd = bvt_bin_path + " -s %s " % partition_size
    try:
        verity_tree_size =  int(subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True).strip())
    except subprocess.CalledProcessError as e:
        bb.debug(1, "cmd: %s" % (cmd))
        bb.fatal("Error in calculating verity tree size: %s\n%s" % (e.returncode, e.output.decode("utf-8")))

    # Get verity metadata size
    bvmd_script_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/build_verity_metadata.py'
    cmd = bvmd_script_path + " size %s " % partition_size
    try:
        verity_metadata_size = int(subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True).strip())
    except subprocess.CalledProcessError as e:
        bb.debug(1, "cmd: %s" % (cmd))
        bb.fatal("Error in calculating verity metadata size: %s\n%s" % (e.returncode, e.output.decode("utf-8")))

    verity_size = verity_tree_size + verity_metadata_size

    # Get fec size
    if fec_support is "1":
        fec_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/fec'
        cmd = fec_bin_path + " -s %s " % (partition_size + verity_size)
        try:
            fec_size =  int(subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True).strip())
        except subprocess.CalledProcessError as e:
            bb.debug(1, "cmd: %s" % (cmd))
            bb.fatal("Error in calculating fec size: %s\n%s" % (e.returncode, e.output.decode("utf-8")))
        d.setVar('FEC_SIZE', str(fec_size))
        return verity_size + fec_size
    return verity_size

make_verity_enabled_system_image[cleandirs] += " ${VERITY_IMAGE_DIR}"

python make_verity_enabled_system_image () {
    import subprocess

    sparse_img = d.getVar('SPARSE_SYSTEM_IMG', True)
    verity_img = d.getVar('VERITY_IMG', True)
    verity_md_img = d.getVar('VERITY_METADATA_IMG', True)
    signer_path = d.getVar('STAGING_BINDIR_NATIVE',True) + "/verity_signer"
    signer_key  = d.getVar('TMPDIR',True) + "/work-shared/security_tools/verity.pk8"

    # Build verity tree
    bvt_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/build_verity_tree'
    cmd = bvt_bin_path + " -A %s %s %s " % (d.getVar("FIXED_SALT",True), sparse_img, verity_img)
    try:
        [root_hash, salt] = (subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)).split()
    except subprocess.CalledProcessError as e:
        bb.debug(1, "cmd %s" % (cmd))
        bb.fatal("Error in building verity tree : %s\n%s" % (e.returncode, e.output.decode("utf-8")))
    d.setVar('ROOT_HASH', root_hash.decode('UTF-8'))
    bb.debug(1, "Value of root hash is %s" % root_hash)
    bb.debug(1, "Value of salt is %s" % salt)

    # Build verity metadata
    blk_dev = d.getVar("BLOCK_DEVICE_SYSTEM",True)
    image_size = d.getVar("SYSTEM_SIZE_EXT4",True)
    bvmd_script_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/build_verity_metadata.py'
    cmd = bvmd_script_path + " build %s %s %s %s %s %s %s " % (image_size, verity_md_img, root_hash, salt, blk_dev, signer_path, signer_key)
    subprocess.call(cmd, shell=True)

    # Append verity metadata to verity image.
    bb.debug(1, "appending verity_img to verity_md_img .... ")
    with open(verity_md_img, "ab") as out_file:
        with open(verity_img, "rb") as input_file:
            for line in input_file:
                out_file.write(line)

    # Calculate padding.
    partition_size = int(d.getVar("ORG_SYSTEM_SIZE_EXT4",True))
    img_size = int(d.getVar("SYSTEM_SIZE_EXT4",True))
    verity_size = int(d.getVar("VERITY_SIZE",True))
    padding_size = partition_size - img_size - verity_size
    bb.debug(1, "padding_size(%s) = %s - %s - %s" %(padding_size, partition_size, img_size, verity_size))
    assert padding_size >= 0

    fec_supported=d.getVar("FEC_SUPPORT",True)
    if fec_supported is "1":
        fec_bin_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/fec'
        fec_img_path = d.getVar('VERITY_FEC_IMG', True)
        cmd = fec_bin_path + " -e -p %s %s %s %s" % (padding_size, sparse_img, verity_md_img, fec_img_path)
        subprocess.call(cmd, shell=True)

        bb.debug(1, "appending fec_img_path to verity_md_img.... ")
        with open(verity_md_img, "ab") as out_file:
            with open(fec_img_path, "rb") as input_file:
                for line in input_file:
                    out_file.write(line)

    # Almost done. Append verity img to sparse system img.
    append2simg_path = d.getVar('STAGING_BINDIR_NATIVE', True) + '/append2simg'
    cmd = append2simg_path + " %s %s " % (sparse_img, verity_md_img)
    subprocess.call(cmd, shell=True)

    #system image is ready. Update verity cmdline.
    dm_prefix = d.getVar('DM_KEY_PREFIX', True)
    dm_key_args_list = []
    dm_key_args_list.append( d.getVar('SIZE_IN_SECTORS', True))
    dm_key_args_list.append( d.getVar('DATA_BLOCKS_NUMBER', True))
    dm_key_args_list.append( str(d.getVar('ROOT_HASH', True)))
    dm_key_args_list.append( d.getVar('FEC_OFFSET', True))
    dm_key =  dm_prefix + " ".join(dm_key_args_list)+ " " +'\\"'
    cmdline = "verity=\\" + dm_key

    bb.debug(1, "Verity Command line set to %s " % (cmdline))

    # Write cmdline to a tmp file
    verity_cmd = d.getVar('VERITY_CMDLINE', True)
    subprocess.check_output("echo '%s' > %s" % (cmdline, verity_cmd), stderr=subprocess.STDOUT, shell=True)

}

def get_verity_cmdline(d):
    import subprocess

    # Get verity cmdline from tmp file
    verity_cmd = d.getVar('VERITY_CMDLINE', True)
    output = subprocess.check_output("grep -m 1 verity %s" % (verity_cmd), shell=True)
    return output.decode('UTF-8')
