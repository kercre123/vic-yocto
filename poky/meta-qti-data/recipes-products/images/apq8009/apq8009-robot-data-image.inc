# Data open source Packages
require apq8009-data-image.inc
IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','robot-rome', 'data', '', d)}"
IMAGE_INSTALL += "${@oe.utils.conditional('PRODUCT','robot-rome', 'iputils', '', d)}"
