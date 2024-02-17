# location open source packages
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','location-services', 'loc-stub', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','location-services', 'loc-pla', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','location-services', 'loc-hal', '', d)}"
IMAGE_INSTALL += "${@bb.utils.contains('DISTRO_FEATURES','location-services', 'loc-net-iface', '', d)}"