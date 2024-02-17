#!/bin/sh

 for uevent in /sys/devices/*.i2c/i2c-*/*-0068/iio:device?*/uevent; do
      . $uevent
      if [ -e $uevent ]; then
        if [ ! -e /dev/iio:device$MINOR ]; then
            mknod /dev/iio:device$MINOR c $MAJOR $MINOR
        fi
      fi
 done
