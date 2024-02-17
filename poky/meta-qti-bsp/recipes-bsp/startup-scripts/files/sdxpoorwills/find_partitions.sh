#!/bin/sh
# Copyright (c) 2014-2015,2018, The Linux Foundation. All rights reserved.
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
# find_partitions        init.d script to dynamically find partitions
#

FindAndMountUBI () {
   partition=$1
   dir=$2
   extra_opts=$3

   mtd_block_number=`cat $mtd_file | grep -i $partition | sed 's/^mtd//' | awk -F ':' '{print $1}'`
   echo "MTD : Detected block device : $dir for $partition"
   mkdir -p $dir

   ubiattach -m $mtd_block_number -d 1 /dev/ubi_ctrl
   device=/dev/ubi1_0
   while [ 1 ]
    do
        if [ -c $device ]
        then
            test -x /sbin/restorecon && /sbin/restorecon $device
            mount -t ubifs /dev/ubi1_0 $dir -o bulk_read$extra_opts
            break
        else
            sleep 0.010
        fi
    done
}

FindAndMountVolumeUBI () {
   volume_name=$1
   dir=$2
   extra_opts=$3
   if [ ! -d $dir ]
   then
       mkdir -p $dir
   fi
   mount -t ubifs ubi0:$volume_name $dir -o bulk_read$extra_opts
}

FindAndMountEXT4 () {
   partition=$1
   dir=$2
   extra_opts=$3
   mmc_block_device=/dev/block/bootdevice/by-name/$partition
   echo "EMMC : Detected block device : $dir for $partition"
   if [ ! -d $dir ]
   then
       mkdir -p $dir
   fi
   mount -t ext4 $mmc_block_device $dir -o relatime,data=ordered,noauto_da_alloc,discard$extra_opts
   echo "EMMC : Mounting of $mmc_block_device on $dir done"
}

emmc_dir=/dev/block/bootdevice/by-name
mtd_file=/proc/mtd

if [ -d $emmc_dir ]
then
        fstype="EXT4"
        eval FindAndMount${fstype} userdata /data
        eval FindAndMount${fstype} cache /cache
        eval FindAndMount${fstype} systemrw /systemrw
        test -x /sbin/restorecon && /sbin/restorecon -RD /data /cache /systemrw
else
        fstype="UBI"
        eval FindAndMountVolume${fstype} usrfs /data
        eval FindAndMountVolume${fstype} systemrw /systemrw
        test -x /sbin/restorecon && /sbin/restorecon -RD /data /systemrw
        eval FindAndMountVolume${fstype} cachefs /cache
	eval FindAndMountVolume${fstype} persist /persist
fi

if [ -x /sbin/restorecon ]; then
    firmware_selinux_opt=",context=system_u:object_r:firmware_t:s0"
else
    firmware_selinux_opt=""
fi
eval FindAndMount${fstype} modem /firmware $firmware_selinux_opt
exit 0
