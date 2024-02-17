#!/bin/sh
# Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

mkdir -p /firmware
mount -t vfat /dev/mmcblk0p1 /firmware

# Check for images and set up symlinks
cd /firmware/image

# Get the list of files in /firmware/image
# for which sym links have to be created

fwfiles=$(ls modem* q6* wcnss* dsps* tzapps* gss*)
modem_fwfiles=$(ls modem_fw.mdt)

# Check if the links with similar names
# have been created in /lib/firmware

mkdir -p /lib/firmware
cd /lib/firmware
linksNeeded=0
fixModemFirmware=0

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

modemfwfile=$(ls $modem_fwfiles)
if [ "$modemfwfile" != "$modem_fwfiles" ]
then
   # file with $modemfile does not exist
   # need to rename the right set of firmware based on chip version
   fixModemFirmware=1
fi

case $linksNeeded in
   1)
      cd /firmware/image

      # Check if need to select modem firmware and do rename in first boot
      case $fixModemFirmware in
      1)
        # Check chip version
        case `cat /sys/devices/system/soc/soc0/version 2>/dev/null` in
          "1.0" | "1.1")
            for file in modem_f1.*
            do
              newname=modem_fw.${file##*.}
              ln -s /firmware/image/$file /lib/firmware/$newname 2>/dev/null
            done
            ;;

          *)
            for file in modem_f2.*
            do
              newname=modem_fw.${file##*.}
              ln -s /firmware/image/$file /lib/firmware/$newname 2>/dev/null
            done
            ;;
         esac
         ;;
      *)
        # Nothing to do.
        ;;
      esac

      case `ls modem.mdt 2>/dev/null` in
         modem.mdt)
            for imgfile in modem*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
        *)
            # trying to log here but nothing will be logged since it is
            # early in the boot process. Is there a way to log this message?
            echo "PIL 8960 device but no modem image found"
            ;;
      esac

      case `ls q6.mdt 2>/dev/null` in
         q6.mdt)
            for imgfile in q6*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL 8960 device but no q6 image found"
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
            echo "PIL 8960 device but no wcnss image found"
            ;;
      esac

      case `ls dsps.mdt 2>/dev/null` in
         dsps.mdt)
            for imgfile in dsps*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL 8960 device but no dsps image found"
            ;;
      esac

      case `ls tzapps.mdt 2>/dev/null` in
         tzapps.mdt)
            for imgfile in tzapps*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "PIL 8960 device but no tzapps image found"
            ;;
      esac

      case `ls gss.mdt 2>/dev/null` in
         gss.mdt)
            for imgfile in gss*
            do
               ln -s /firmware/image/$imgfile /lib/firmware/$imgfile 2>/dev/null
            done
            ;;
         *)
            echo "No gss image found"
            ;;
      esac
      ;;

   *)
      echo "Nothing to do. No firmware links needed."
      ;;
esac

cd /firmware/image

ln -s /firmware/image/apps.mbn /lib/firmware/apps.mbn 2>/dev/null
ln -s /firmware/image/dsp1.mbn /lib/firmware/dsp1.mbn 2>/dev/null
ln -s /firmware/image/dsp2.mbn /lib/firmware/dsp2.mbn 2>/dev/null
ln -s /firmware/image/dsp3.mbn /lib/firmware/dsp3.mbn 2>/dev/null
ln -s /firmware/image/rpm.mbn  /lib/firmware/rpm.mbn  2>/dev/null
ln -s /firmware/image/sbl1.mbn /lib/firmware/sbl1.mbn 2>/dev/null
ln -s /firmware/image/sbl2.mbn /lib/firmware/sbl2.mbn 2>/dev/null
ln -s /firmware/image/efs1.mbn /lib/firmware/efs1.mbn 2>/dev/null
ln -s /firmware/image/efs2.mbn /lib/firmware/efs2.mbn 2>/dev/null
ln -s /firmware/image/efs3.mbn /lib/firmware/efs3.mbn 2>/dev/null
ln -s /firmware/image/acdb.mbn /lib/firmware/acdb.mbn 2>/dev/null
ln -s /firmware/image/mdm_acdb.img /lib/firmware/mdm_acdb.img 2>/dev/null

cd /
exit 0
