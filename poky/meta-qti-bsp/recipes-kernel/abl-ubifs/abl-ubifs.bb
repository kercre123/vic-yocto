require recipes-kernel/edk2/edk2_git.bb

PROVIDES_remove = "virtual/bootloader"

EXTRA_OEMAKE_remove = " 'NAND_SQUASHFS_SUPPORT=${NAND_SQUASHFS_SUPPORT}'"

do_deploy() {
     if ${@bb.utils.contains('IMAGE_FSTYPES', 'ubi', 'true', 'false', d)}; then
        install -d  ${DEPLOY_DIR_IMAGE_NAND}
        install ${D}/boot/abl.elf ${DEPLOY_DIR_IMAGE_NAND}/abl-ubifs.elf
     fi
}
