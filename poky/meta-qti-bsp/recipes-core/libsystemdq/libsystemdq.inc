SUMMARY = "A System and service manager"
HOMEPAGE = "http://www.freedesktop.org/wiki/Software/systemd"

DESCRIPTION = "libsystemdq is library for systemd  utility library"

LICENSE = "LGPLv2.1"
LIC_FILES_CHKSUM = "file://LICENSE.GPL2;md5=751419260aa954499f7abaabaa882bbe \
                    file://LICENSE.LGPL2.1;md5=4fbd65380cdd255951079008b364516c"

SRCREV = "de7436b02badc82200dc127ff190b8155769b8e7"

SRC_URI = "${CLO_LE_GIT}/systemd.git;protocol=https;branch=caf_migration/systemd/main"

S = "${WORKDIR}/git"
