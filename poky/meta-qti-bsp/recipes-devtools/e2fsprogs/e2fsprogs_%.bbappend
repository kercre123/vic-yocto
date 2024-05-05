#Below Package is fetch from Codelinaro 

SRC_URI = "${CLO_LE_GIT}/e2fsprogs.git;protocol=https;branch=caf_migration/ext2/master"
SRC_URI += "file://remove.ldconfig.call.patch \
            file://run-ptest \
            file://ptest.patch \
            file://Revert-mke2fs-enable-the-metadata_csum-and-64bit-fea.patch \
            file://mkdir_p.patch \
            file://0001-misc-create_inode.c-set-dir-s-mode-correctly.patch \
            file://0001-create_inode-fix-copying-large-files.patch \
            "
SRC_URI[md5sum] = "cf9b4eaa7eda43f39d962c09c1b06709"
SRC_URI[sha256sum] = "3e0a1b5e4de39f9fc320a831bd2c10e9d97b781020995335352e999de7f13658"

