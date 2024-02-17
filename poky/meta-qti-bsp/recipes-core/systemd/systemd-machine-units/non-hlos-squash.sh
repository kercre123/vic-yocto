#!/bin/sh
# Copyright (c) 2019, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#   * Neither the name of The Linux Foundation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
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
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE

# Changes from Qualcomm Innovation Center are provided under the following
# license:
#
# Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause-Clear

IsFirmwareMounted () {
    firmware_mounted=`cat /proc/mounts | grep -i "firmware squashfs" | wc -l`
    if [ "$firmware_mounted" == 1 ] ; then
        echo "firmware volume is already mounted" > /dev/kmsg
        exit 0;
    fi
}

GetFirmwareVolumeID () {
    firmware=$1
    act_slot=`cat /proc/cmdline | sed 's/.*SLOT_SUFFIX=//' | awk '{print $1}'`
    firmware_ab_name=${firmware}${act_slot}
    volcount=`cat /sys/class/ubi/ubi0/volumes_count`
    echo "find ubi index for firmware"  > /dev/kmsg
    for vid in `seq 0 $volcount`; do
        echo $vid  > /dev/kmsg
        name=`cat /sys/class/ubi/ubi0_$vid/name`
        if [ "$name" == "$firmware" ] || [ "$name" == "$firmware_ab_name" ]; then
            echo $vid
            break
        fi
        echo $name  > /dev/kmsg
    done
}

FindAndMountUBIVol () {
   partition=$1
   dir=$2

   volid=$(GetFirmwareVolumeID $partition)
   echo "found volume index for firmware mount " $volid  > /dev/kmsg
   if [ "$volid" == "" ]; then
       return
   fi

   device=/dev/ubi0_$volid
   block_device=/dev/ubiblock0_$volid
   mkdir -p $dir

   if [ -e "/dev/ubiblock0_$volid" ]; then
        echo "/dev/ubiblock0_$volid exists" > /dev/kmsg
   else
        echo "/dev/ubiblock0_$volid desnt exists, creating" > /dev/kmsg
        ubiblock --create $device
   fi

   mount -t squashfs $block_device $dir -o ro
   if [ $? -ne 0 ] ; then
      echo "Unable to mount squashfs onto $block_device."
      mtd_device=`cat /proc/mtd | grep nand_ab_attr | awk -F ':' '{print $1}'`
      chmod 664 /dev/${mtd_device}
      boot_slot=$(/bin/sh -c '/sbin/abctl --boot_slot')
      echo " boot_slot $boot_slot " > /dev/kmsg
      firmware_ab_name=$(cat /sys/class/ubi/ubi0_${volid}/name)
      echo " firmware_ab_name $firmware_ab_name " > /dev/kmsg
      if [ "$firmware_ab_name" == "firmware_a" ] || [ "$firmware_ab_name" == "firmware_b" ] ; then
          echo " a/b volumes " > /dev/kmsg
          /bin/sh -c '/usr/bin/nad-abctl --check_bl_cookie'
          if [ "$?" == "1" ]; then
              echo "cookie is Bootloader switch to edl mode" > /dev/kmsg
              /bin/sh -c 'reboot edl'
          else
              echo "cookie is not bootloader, switch slot and reboot" > /dev/kmsg
              if [ "$boot_slot" == "_a" ] ; then
                  echo " set _b as active slot " > /dev/kmsg
                  /bin/sh -c '/usr/bin/nad-abctl --set_active 1'
              else
                  echo " set _a as active slot " > /dev/kmsg
                  /bin/sh -c '/usr/bin/nad-abctl --set_active 0'
              fi
              /bin/sh -c 'reboot'
          fi
      else
          echo " non a/b volumes , reboot to edl " > /dev/kmsg
          /bin/sh -c 'reboot edl'
      fi
      exit 1
   fi
}

FindAndMountUBI () {
   partition=$1
   dir=$2

   mtd_block_number=`cat $mtd_file | grep -i $partition | sed 's/^mtd//' | awk -F ':' '{print $1}'`
   echo "MTD : Detected block device : $dir for $partition"
   mkdir -p $dir

   ubiattach -m $mtd_block_number
   non_hlos_block=`cat $mtd_file | grep -i nonhlos-fs | sed 's/^mtd//' | awk -F ':' '{print $1}'`
   device=/dev/mtdblock$non_hlos_block
   while [ 1 ]
    do
        if [ -b $device ]
        then
            mount $device /firmware
            break
        else
            sleep 0.010
        fi
    done
}

mtd_file=/proc/mtd
nad_ubi_present=`cat $mtd_file | grep nad_ubi | wc -l`

if [ $nad_ubi_present -eq 0 ]; then
   eval FindAndMountUBI modem /firmware
else
   IsFirmwareMounted
   eval FindAndMountUBIVol firmware /firmware
   if [ $? -ne 0 ] ; then
      echo "Unable to mount firmware volume" > /dev/kmsg
      exit -1
   fi
   echo "firmware volume is sucessfully mounted " > /dev/kmsg
fi
exit 0
