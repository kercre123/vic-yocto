DESCRIPTION = "Minimal image with SELinux support (no python)"

IMAGE_FEATURES += "splash ssh-server-openssh"

LICENSE = "MIT"

IMAGE_INSTALL = "\
	${CORE_IMAGE_BASE_INSTALL} \
	bash \
	util-linux-agetty \
	packagegroup-core-boot \
	packagegroup-selinux-minimal \
"

inherit selinux-image
