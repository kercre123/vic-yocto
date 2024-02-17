inherit native autotools

MY_PN = "mkyaffs2image"

DESCRIPTION = "YAFFS (Yet Another Flash File System) provides a fast robust file system for NAND and NOR Flash. "
HOMEPAGE = "http://www.yaffs.net/"
LICENSE = "GPLv2"
# FIXME - This is using the whole thing here and the license grant's in the comments only...
LIC_FILES_CHKSUM = "file://yaffs2/utils/mkyaffs2image.c;md5=8e9d7c91f4196c80b7ab23635580ca7a"
EXTRA_OECONF = "--with-core-includes=${WORKSPACE}/system/core/include"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://external/yaffs2/"
S = "${WORKDIR}/external/yaffs2"

PR = "r3"
