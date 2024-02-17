#
# Common bitbake recipe information for QTI meta layers.
# Below are common values, statements and functions.
#
inherit autotools-brokensep pkgconfig

FILESPATH        =+ "${WORKSPACE}:"

SRC_URI          = "file://${@d.getVar('SRC_DIR', True).replace('${WORKSPACE}/', '')}"

PACKAGE_ARCH    ?= "${MACHINE_ARCH}"
