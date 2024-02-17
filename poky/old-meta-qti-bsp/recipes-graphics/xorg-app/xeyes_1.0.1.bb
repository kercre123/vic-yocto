require recipes-graphics/xorg-app/xorg-app-common.inc
PE = "1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=4641deddaa80fe7ca88e944e1fd94a94"
SRC_URI[md5sum] = "033f14f7c4e30d1f4edbb22d5ef86883"

DEPENDS += " virtual/libx11 libxau libxt libxext libxmu"
