DESCRIPTION = "Image with SELinux support" 

IMAGE_FEATURES += "splash ssh-server-openssh"

LICENSE = "MIT"

IMAGE_INSTALL = "\
	${CORE_IMAGE_BASE_INSTALL} \
	util-linux-agetty \
	packagegroup-core-full-cmdline \
	packagegroup-core-selinux \
"   

inherit selinux-image
