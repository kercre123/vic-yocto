#!/bin/sh
# Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# find_recovery_partitions        init.d script to dynamically find partitions used in recovery
#

emmc_dir=/dev/block/bootdevice/by-name
mtd_file=/proc/mtd
fstab_file=/tmp/recovery_volume_detected


ubi_device_number=1

UpdateRecoveryVolume () {
   partition=$1
   dir=$2
   fs_type=$3
   device=$4
   echo "$device       $dir     $fs_type     defaults    0   0" >> $fstab_file
}

FindAndAttachUBI() {
   partition=$1
   num_volumes=$2

   mtd_block_number=`cat $mtd_file | grep -i $partition | sed 's/^mtd//' | awk -F ':' '{print $1}'`
   if [ -z "$mtd_block_number" ]; then
      echo "MTD : Partition $partition not found" > /dev/kmsg
   else
      echo "MTD : Attaching UBI device /dev/mtdblock$mtd_block_number for $partition @$ubi_device_number" > /dev/kmsg

      ubiattach -m $mtd_block_number -d $ubi_device_number /dev/ubi_ctrl
      count=1000 # wait for the ubi-device node to be created for a max of (1000 * 0.010 = ) 10 seconds
      while [ 1 ]; do
         if [ -c /dev/ubi${ubi_device_number} ]; then
            echo "/dev/ubi${ubi_device_number} created" > /dev/kmsg
            break
         else
            count=$(($count - 1))
            if [ $count -lt 1 ]; then
               echo "/dev/ubi${ubi_device_number} not yet created, rebooting" > /dev/kmsg
               /sbin/reboot
               break
            else
               sleep 0.010
            fi
         fi
      done

      i=0
      while [ "$i" -lt "$num_volumes" ]; do
         count=500 # wait for the ubi volume node to be created, for max 5 seconds
         volume_node="/dev/ubi${ubi_device_number}_${i}"
         while [ 1 ]; do
            if [ -c $volume_node ]; then
               echo "$volume_node created, proceeding" > /dev/kmsg
               break
            else
               count=$(($count - 1))
               if [ $count -lt 1 ]; then
                  echo "$volume_node not yet created, rebooting .." > /dev/kmsg
                  /sbin/reboot
                  break
               else
                  echo "$volume_node not yet created, checking again .." > /dev/kmsg
                  sleep 0.010
               fi
            fi
         done
         i=$(($i + 1))
      done
      ubi_device_number=$(($ubi_device_number + 1))
   fi

   ls -al /dev | grep -i "ubi" > /dev/kmsg
}

FindAndMountUBI () {
   volume=$1
   dir=$2
   fstab_only="$3"

   echo "MTD : Looking for UBI volume : $dir for $volume" > /dev/kmsg
   mkdir -p $dir

   # Skip ubi0 for recoveryfs
   for ubidev in /dev/ubi[1-99]_*; do
      volname=`ubinfo $ubidev | grep Name\: | awk '{print $2}'`
      if [ "$volname" == "$volume" ]; then
         echo "Found Volume: $volname" > /dev/kmsg
         if [ "$fstab_only" != "1" ]; then
            mount -t ubifs $ubidev $dir -o bulk_read
            echo "MTD : Mounting of $ubidev on $dir done" > /dev/kmsg
         fi
         UpdateRecoveryVolume $volume $dir "ubifs" $ubidev
         break
      fi
   done
}

FindAndMountEXT4 () {
   partition=$1
   dir=$2
   fstab_only="$3"

   mmc_block_device=/dev/block/bootdevice/by-name/$partition

   echo "EMMC : Looking for EXT4 block device : $dir for $partition"
   mkdir -p $dir
   if [ "$fstab_only" != "1" ]; then
      mount -t ext4 $mmc_block_device $dir -o relatime,data=ordered,noauto_da_alloc,discard
      echo "EMMC : Mounting of $mmc_block_device on $dir done"
   fi
   UpdateRecoveryVolume $1 $2 "ext4" $mmc_block_device
}

FindAndMountMTD () {
   partition=$1
   dir=$2

   mtd_block_device=`cat /proc/mtd | grep -i $partition | sed 's/^mtd/mtdblock/' | awk -F ':' '{print $1}'`
   echo "Detected block device : $dir for $partition" > /dev/kmsg
   mkdir -p $dir
   mount -t mtd /dev/$mtd_block_device $dir
   echo "Mounting of /dev/$mmc_block_device on $dir done" > /dev/kmsg

   UpdateRecoveryVolume $1 $2 "mtd" /dev/$mtd_block_device
}

echo -n > $fstab_file

if [ -d $emmc_dir ]
then
    fstype="EXT4"
    eval FindAndMountEXT4 system   /system   1
    eval FindAndMountEXT4 userdata /data     1
    eval FindAndMountEXT4 cache    /cache
else
    fstype="UBI"
    eval FindAndAttachUBI system 4
    eval FindAndMountUBI rootfs  /system  1
    eval FindAndMountUBI usrfs   /data    1
    eval FindAndMountUBI cachefs /cache
fi

FindAndMountMTD misc /misc
# set selinux to permissive mode before we start recovery executable
/usr/sbin/setenforce 0

exit
