SUMMARY = "Linux Bluetooth Stack Userland V5"
DESCRIPTION = "Linux Bluetooth stack V5 userland components.  These include a system configurations, daemons, tools and system libraries."
HOMEPAGE = "http://www.bluez.org"
SECTION = "libs"
LICENSE = "GPLv2+ & LGPLv2.1+"

LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e \
                    file://COPYING.LIB;md5=fb504b67c50331fc78734fed90fb0e09 \
                    file://src/main.c;beginline=1;endline=24;md5=9bc54b93cd7e17bf03f52513f39f926e"

DEPENDS = "udev alsa-lib libusb dbus-glib glib-2.0 libical"

ASNEEDED = ""

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://external/bluetooth/bluez/"

S = "${WORKDIR}/external/bluetooth/bluez/"

inherit autotools-brokensep pkgconfig

EXTRA_OECONF = "\
  --enable-tools \
  --disable-cups \
  --enable-test \
  --enable-datafiles \
  --disable-systemd \
  --with-glib \
"
#  unused param
#  --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
