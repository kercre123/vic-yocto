#!/bin/sh

# create symbolic links for vendor/libs
if [ ! -d /system/vendor/lib ] ; then
  mkdir -p /system/vendor/lib
fi

ln -s /system/vendor/lib /system/lib
ln -s /system/vendor /vendor
