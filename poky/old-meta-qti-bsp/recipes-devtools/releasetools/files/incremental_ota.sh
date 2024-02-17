#!/bin/sh
# Copyright (c) 2017, The Linux Foundation. All rights reserved.
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
# incremental_ota.sh     script to generate OTA upgrade pacakges.
#

set -o xtrace

if [ "$#" -ne 4 ]; then
    echo "Usage  : $0 v1_target_files_zipfile v2_target_files_zipfile rootfs_path ext4_or_ubi"
    echo "----------------------------------------------------------------------------------------------------"
    echo "example: $0 v1_target_files_ubi.zip  v2_target_files_ubi.zip  machine-image/1.0-r0/rootfs ubi"
    echo "example: $0 v1_target_files_ext4.zip v2_target_files_ext4.zip machine-image/1.0/rootfs ext4"
    exit 1
fi

export PATH=.:${STAGING_BINDIR_NATIVE}:$PATH
export OUT_HOST_ROOT=.
export LD_LIBRARY_PATH=${STAGING_LIBDIR_NATIVE}
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8
export LANGUAGE=en_US.UTF-8

# Specify MMC or MTD type device. MTD by default
[[ $4 = "ext4" ]] && device_type="MMC" || device_type="MTD"

rm -rf target_files
unzip -qo $2 -d target_files
mkdir -p target_files/META
mkdir -p target_files/SYSTEM

# Generate selabel rules only if file_contexts is packed in target-files
if grep "selinux_fc" target_files/META/misc_info.txt
then
    zipinfo -1 $2 |  awk 'BEGIN { FS="SYSTEM/" } /^SYSTEM\// {print "system/" $2}' | fs_config -C -S target_files/BOOT/RAMDISK/file_contexts -D ${3} > target_files/META/filesystem_config.txt
else
    zipinfo -1 $2 |  awk 'BEGIN { FS="SYSTEM/" } /^SYSTEM\// {print "system/" $2}' | fs_config -D ${3} > target_files/META/filesystem_config.txt
fi


cd target_files && zip -q ../$2 META/*filesystem_config.txt SYSTEM/build.prop && cd ..

python3 ./ota_from_target_files -n -v -d $device_type -v -p . -s "${WORKSPACE}/android_compat/device/qcom/common" --no_signing -i $1 $2 update_incr_$4.zip
