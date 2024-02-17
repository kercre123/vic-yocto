FILESEXTRAPATHS_prepend := "${THISDIR}/systemd:"

SRC_URI += "file://1001-systemd-Disable-unused-mount-points.patch"
SRC_URI += "file://1002-systemd-Use-data-etc-localtime.patch"
SRC_URI += "file://1003-anki-Backport-env-status-for-ExecStop-ExecStopPost-c.patch"
SRC_URI += "file://1004-capabilities-keep-bounding-set-in-non-inverted-forma.patch"
SRC_URI += "file://1005-capabilities-added-support-for-ambient-capabilities.patch"
SRC_URI += "file://mountpartitions.rules"
SRC_URI += "file://systemd-udevd.service"
SRC_URI += "file://ffbm.target"
SRC_URI += "file://mtpserver.rules"
SRC_URI += "file://setup_localtime_link"
SRC_URI += "file://setup_localtime_link.service"

DEPENDS += "blkdiscard"

EXTRA_OECONF += " --disable-efi"

# In aarch64 targets systemd is not booting with -finline-functions -finline-limit=64 optimizations
# So temporarily revert to default optimizations for systemd.
FULL_OPTIMIZATION = "-O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer -ftree-vectorize"