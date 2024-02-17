SUMMARY = "SELinux init script"
DESCRIPTION = "Set SELinux labels for /dev."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

${PN}_RDEPENDS = " \
    coreutils \
    libselinux-bin \
    policycoreutils-setfiles \
"

SRC_URI = "file://${BPN}.sh \
		file://${BPN}.service \
	"

SELINUX_SCRIPT_DST = "0${BPN}"

require selinux-initsh.inc
