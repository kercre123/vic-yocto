#packages installed to root file system of PSM build
require mdm9607-base-image.inc

PACKAGE_EXCLUDE += "openssl"

PACKAGE_EXCLUDE += "wireless-tools"
#Excluding Flouride BT packages
PACKAGE_EXCLUDE += "packagegroup-bt"
PACKAGE_EXCLUDE += "bt-app"
PACKAGE_EXCLUDE += "bt-property"
PACKAGE_EXCLUDE+= "fluoride"
PACKAGE_EXCLUDE += "libbt-vendor"

PACKAGE_EXCLUDE += "start-scripts-init-can"
