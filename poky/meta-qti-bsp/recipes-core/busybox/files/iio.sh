#!/bin/sh

 for uevent in /sys/bus/i2c/devices/*-006*/iio:device?*/uevent; do
      . $uevent
    chown root:sensors /sys/bus/iio/devices
    chmod 755 /sys/bus/iio/devices
    if [ -e $uevent ]; then
        if [ ! -e /dev/iio:device$MINOR ]; then
            mknod /dev/iio:device$MINOR c $MAJOR $MINOR
            chown -R root:sensors /dev/iio:device$MINOR
            chown -R root:sensors /sys/bus/iio/devices/iio:device$MINOR
            chown -R root:sensors /sys/bus/iio/devices/iio:device$MINOR/*
            #Give Executive permissions for all folders
            chmod 755 $(find /sys/bus/iio/devices/iio:device$MINOR -type d)
            #Give read and write permissions for group and only read for otheres
            chmod 664 $(find /sys/bus/iio/devices/iio:device$MINOR/ -type f)
        fi
      fi
 done
