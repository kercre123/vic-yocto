#!/bin/sh
# Copyright (c) 2016, The Linux Foundation. All rights reserved.
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
# firmware-links        init.d script to install the firmware links
#

# No path is set up at this point so we have to do it here.
PATH=/sbin:/bin:/usr/sbin:/usr/bin
export PATH

slot_suffix=$(getslotsuffix)

if [ -f /etc/selinux/config ];then
   mount -t vfat /dev/block/bootdevice/by-name/modem${slot_suffix} /firmware -o context=system_u:object_r:firmware_t:s0,noexec,nodev,ro,gid=1000
else
   mount -t vfat /dev/block/bootdevice/by-name/modem${slot_suffix} /firmware -o noexec,nodev,ro,gid=1000
fi


# Check for images and set up symlinks
cd /firmware/image

# Get the list of files in /firmware/image
# for which sym links have to be created

fwfiles=$(ls modem* adsp* wcnss* *.bin mba* venus*)

# Check if the links with similar names
# have been created in /lib/firmware

mkdir -p /lib/firmware
cd /lib/firmware
linksNeeded=0

# For everyfile in fwfiles check if
# the corresponding file exists
for fwfile in $fwfiles
do
   # if (condition) does not seem to work
   # with the android shell. Therefore
   # make do with case statements instead.
   # if a file named $fwfile is present
   # no need to create links. If the file
   # with the name $fwfile is not present
   # need to create links.

   fw_file=$(ls $fwfile)
   if [ "$fw_file" == "$fwfile" ]
   then
      continue
   else
      linksNeeded=1
      break
   fi
done

case $linksNeeded in
   1)
      cd /firmware/image

      case `ls modem.mdt 2>/dev/null` in
         modem.mdt)
            for imgfile in mba.mbn modem*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
        *)
            # trying to log here but nothing will be logged since it is
            # early in the boot process. Is there a way to log this message?
            echo "PIL no modem image found"
            ;;
      esac

      case `ls adsp.mdt 2>/dev/null` in
         adsp.mdt)
            for imgfile in adsp*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL no adsp image found"
            ;;
      esac

      case `ls cpe_9335.mdt 2>/dev/null` in
         cpe_9335.mdt)
            for imgfile in cpe_*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
        *)
            # trying to log here but nothing will be logged since it is
            # early in the boot process. Is there a way to log this message?
            echo "PIL no cpe_9335 image found"
            ;;
      esac

      case `ls wcnss.mdt 2>/dev/null` in
         wcnss.mdt)
            for imgfile in wcnss*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL no wcnss image found"
            ;;
      esac

      case `ls mba.mdt 2>/dev/null` in
         mba.mdt)
            for imgfile in mba*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL no mba image found"
            ;;
      esac

      case `ls venus.mdt 2>/dev/null` in
         venus.mdt)
            for imgfile in venus*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL no venus image found"
            ;;
      esac
	  ;;
   *)
      echo "Nothing to do. No firmware links needed."
      ;;
esac

cd /

# Create a Symbolic Links for WCNSS_qcom_wlan_nv.bin and WCNSS_wlan_dictionary.dat firmware files
# which are stored at /persist partition.
ln -s /persist/WCNSS_qcom_wlan_nv.bin /lib/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin
ln -s /persist/WCNSS_wlan_dictionary.dat /lib/firmware/wlan/prima/WCNSS_wlan_dictionary.dat


exit 0
