SUMMARY = "SELinux init script"
DESCRIPTION = "\
Script to detect and attempt to correct a misconfigured SELinux system at \
boot time. \
"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

${PN}_RDEPENDS = " \
    coreutils \
    libselinux-bin \
    policycoreutils-secon \
    policycoreutils-setfiles \
"

SRC_URI = "file://${BPN}.sh \
		file://${BPN}.service \
	"

INITSCRIPT_PARAMS = "start 01 S ."

require selinux-initsh.inc
