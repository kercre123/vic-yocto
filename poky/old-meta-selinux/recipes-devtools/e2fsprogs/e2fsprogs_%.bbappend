FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

DEPENDS += "linux-libc-headers attr"

SRC_URI += " \
    file://misc-xattr-add-xattr-module-stub.patch \
    file://mke2fs.c-create_inode.c-copy-xattrs.patch \
    file://lib-ext2fs-ext2_ext_attr.h-add-xattr-index.patch \
    file://misc-xattr-create-xattr-block.patch \
    file://misc-xattr-create-xattr-block-node.patch \
    file://misc-xattr-create-xattr-cache.patch \
"
