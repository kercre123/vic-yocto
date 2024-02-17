#!/bin/sh
# Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
# recovery-updater
# This script will check if recovery/recoveryfs partitions need
# an upgrade. It needs an update package(.zip) which has a recovery image
# and an unsparsed recoveryfs image packed within.
#

# The workspace for recovery-updater
cache_work_dir="/cache/recoveryupgrade"
mkdir -p $cache_work_dir

# Redirect stdout to "log" in /cache/recoveryupgrade
exec > $cache_work_dir/log

# Redirect stderr also to the same log file
exec 2>&1

date=`date`
self_pid=`echo $$`
# Fetch own user:group info by stat'ing /proc/<pid> dir
credentials=`stat --printf=User:%U,\ Group:%G /proc/$self_pid`

INIT_LOG="\nrecovery-updater firing up.
          \nDATE&TIME -  $date
          \nProcess ID - $self_pid
          \nCredentials - $credentials \n"

echo -e $INIT_LOG

# Cookie to indicate whether to perform recovery-upgrade or not
cookie="${cache_work_dir}/RECOVERY_UPGRADE_DONE"

# By default, we assume that recovery-upgrade is not needed
recovery_upgrade_needed=0
recoveryfs_upgrade_needed=0

# Initialize recoveryfs image global variables
recoveryfs_partition="/dev/block/bootdevice/by-name/recoveryfs"
recoveryfsimg_sha1=`cat /recoveryupgrade/recoveryfsimg_sha1`
recoveryfsimg_length=`cat /recoveryupgrade/recoveryfsimg_length`
recoveryfsimg_sha1_file="/recoveryupgrade/recoveryfsimg_sha1"
recoveryfsimg_length_file="/recoveryupgrade/recoveryfsimg_length"

# Initialize recovery image global variables
recovery_partition="/dev/block/bootdevice/by-name/recovery"
recoveryimg_sha1=`cat /recoveryupgrade/recoveryimg_sha1`
recoveryimg_length=`cat /recoveryupgrade/recoveryimg_length`
recoveryimg_sha1_file="/recoveryupgrade/recoveryimg_sha1"
recoveryimg_length_file="/recoveryupgrade/recoveryimg_length"

# Initialize upgrade package global variables
recoveryimg="recoveryupgrade/recovery.img"
recoveryfsimg="recoveryupgrade/recoveryfs-unsparsed.img"

# Global state variable to indicate if
# upgrade package (.zip) passes sanity or not
upgrade_zip_sanity_failed=0 # assume sanity by default

# Important: We assume that the only zip file in
# /cache has to be the upgrade package.
zipfile=`ls /cache/*.zip | awk '{ print $1 }'`

# As a sanity check, count the number of *.zip
# in the said location. If > 1, exit!
count_of_zipfiles=`ls -1 /cache/*.zip | wc -l`
if [ $count_of_zipfiles -gt 1 ];
then
  echo -e "\nMore than one zip file present. Aborting!"
  echo -e "\nFound - $zipfile"
  upgrade_zip_sanity_failed=1 # sanity failed
fi


################################################################################
# Helper functions

# Writes RECOVERY_UPGRADE_DONE cookie
WriteRecoveryUpgradeDoneCookie() {
  echo -e "Writing RECOVERY_UPGRADE_DONE cookie ..."
  echo "RECOVERY_UPGRADE_DONE" > $cookie
  cat $cookie
}

# Takes as argument any "exit code" from another command
# and terminates; In case of an abnormal exit code, If the
# second argument is 1, the cookie is written before terminating.
CheckExitCodeAndTerminate() {
  if [ $1 != 0  ];
  then
    echo -e "\nObserved an abnormal exit code. Exiting ..."
    # For an abnormal exit code, provide the option of writing the cookie
    if [ $2 == 1 ];
    then
      WriteRecoveryUpgradeDoneCookie
    fi
    exit 1
  fi
}

# Reads upto 'input_length' bytes from 'input_stream'
# and then computes sha1sum; which is then compared against
# 'sha1reference'. Returns 0 if sha1sums match.
CompareSHA1Sum() {
  input_stream=$1
  input_length=$2
  sha1reference=$3

  echo -e "\nComparing sha1sum of $input_stream against $sha1reference"

  # If input_length is > 0, compute sha1sum for only that length
  # of input_stream.
  # Else, compute sha1sum for the whole input_file.
  if [ $input_length -gt 0 ];
  then
    sha1sum=`head -c $input_length $input_stream  | sha1sum | awk '{ print $1 }'`
  else
    sha1sum=`sha1sum $input_stream | awk '{ print $1 }'`
  fi

  echo -e $sha1sum "vs." $sha1reference
  [ "$sha1sum" == "$sha1reference" ]
}

# Unzip 'file' file from 'zip' zipfile and write
# the contents to 'output_stream'.
# Terminates the program (without writing the cookie)
# if any of the operations within encountered errors.
UpgradePartitionWithImage() {
  output_stream=$1
  zip=$2
  file=$3

  # Zero out the partition before updating new content
  echo -e "\nZeroing out $output_stream ..."
  dd if=/dev/zero of=$output_stream bs=8192 conv=fsync

  # Use dd to update the partition with the input image
  echo -e "\nUpgrading $output_stream with $input_stream ..."
  unzip -p $zip $file > $output_stream
  CheckExitCodeAndTerminate $? 0
}

# Unzip 'file_in_zip' from 'zip' and compare the
# the length of the extracted content against 'reference',
# which is a file containing reference length.
# Returns 0 if lengths match.
UnzipAndCompareLength() {
  zip=$1
  file_in_zip=$2
  reference=$3 # File containing reference length
  temp_file1="${cache_work_dir}/temp1"
  temp_file2="${cache_work_dir}/temp2"

  # Remove  '\n' and ' ' from content to be diff'ed
  unzip -p $zip $file_in_zip | wc -c | tr -d '\n ' > $temp_file1
  cat $reference | tr -d '\n ' > $temp_file2

  diff $temp_file1 $temp_file2
  ret_code=$?

  rm $temp_file1 $temp_file2 # Clean up temp files
  return $ret_code
}

# Unzip 'file_in_zip' from 'zip' and compare the
# the sha1sum of the extracted content against 'reference',
# which is a file containing reference sha1sum.
# Returns 0 if sha1sums match.
UnzipAndCompareSHA1Sum() {
  zip=$1
  file_in_zip=$2
  reference=$3 # File containing reference sha1sum
  temp_file1="${cache_work_dir}/temp1"
  temp_file2="${cache_work_dir}/temp2"

  # Remove  '\n' and ' ' from content to be diff'ed
  unzip -p $zip $file_in_zip | sha1sum | awk '{ print $1 }' | \
          tr -d '\n ' > $temp_file1
  cat $reference | tr -d '\n ' > $temp_file2

  diff $temp_file1 $temp_file2
  ret_code=$?

  rm $temp_file1 $temp_file2 # Clean up temp files
  return $ret_code
}
# End of Helper functions
################################################################################

# Checks the sanity of 'zip' - upgrade package.
ExtractAndCheckUpgradePackage() {
  zip=$1

  echo -e "\nChecking zip file's sanity ..."

  # Check if the upgrade package actually exists or not.
  if [ "$zip" == "" ];
  then
    echo -e "\nCouldn't find any upgrade package in /cache !"
    upgrade_zip_sanity_failed=1
  fi

  echo -e "\nFound $zip ..."

  # We have an update package,
  # Perform some sanity checks on it.
  #
  # Extract the recoveryimg/recoveryfsimg from the zip
  # and check if the length&sha1sum of the images are the
  # same as the ones in /recoveryupgrade.
  #
  # Note that if any of the sanity checks fail,
  # it means that we have lost our confidence on the
  # existing update package.

  echo -e "\nChecking sanity of recovery-images from zip ..."

  if UnzipAndCompareSHA1Sum $zip $recoveryfsimg $recoveryfsimg_sha1_file;
  then
    echo -e "\nsha1sum of recoveryfsimg from zip seems fine"
  else
    echo -e "\nsha1sum of recoveryfsimg from zip didn't match the reference!"
    upgrade_zip_sanity_failed=1
  fi

  if UnzipAndCompareSHA1Sum $zip $recoveryimg $recoveryimg_sha1_file;
  then
    echo -e "\nsha1sum of recoveryimg from zip seems fine"
  else
    echo -e "\nsha1sum of recoveryimg from zip didn't match the reference!"
    upgrade_zip_sanity_failed=1
  fi

  if UnzipAndCompareLength $zip $recoveryfsimg $recoveryfsimg_length_file;
  then
    echo -e "\nLength of recoveryfsimg from zip seems fine"
  else
    echo -e "\nLength of recoveryfsimg from zip didn't match the reference!"
    upgrade_zip_sanity_failed=1
  fi

  if UnzipAndCompareLength $zip $recoveryimg $recoveryimg_length_file;
  then
    echo -e "\nLength of recoveryimg from zip seems fine"
  else
    echo -e "\nLength of recoveryimg from zip didn't match the reference!"
    upgrade_zip_sanity_failed=1
  fi

  if [ "$upgrade_zip_sanity_failed" == 0 ];
  then
    echo -e "\nSanity checks on the upgrade package look promising."
    echo -e "Good to go with recovery-upgrade!"
  else
    echo -e "\nThe zip file has failed some sanity checks!!!"
  fi

}

# Don't bother with recovery(fs) upgrade
# if a cookie already exists in /cache/recoveryupgrade.
# When an OTA upgrade is triggered, recovery module
# will clear this cookie signalling us to do an explicit upgrade.
if [ -e $cookie ];
then
  echo -e "\nA cookie is present in /cache/recoveryupgrade."
  echo -e "We must have already done a recovery-upgrade recently. Nothing to do!"
  # Recreate the cookie, just in case
  WriteRecoveryUpgradeDoneCookie
else
  # Check whether recovery partition needs an upgrade?
  if CompareSHA1Sum $recovery_partition $recoveryimg_length $recoveryimg_sha1;
  then
    echo -e "\nRecovery partition doesn't need an upgrade."
  else
    recovery_upgrade_needed=1
    echo -e "\nRecovery partition needs an upgrade."
  fi

  if CompareSHA1Sum $recoveryfs_partition $recoveryfsimg_length $recoveryfsimg_sha1;
  then
    echo -e "\nRecoveryfs partition doesn't need an upgrade."
  else
    recoveryfs_upgrade_needed=1
    echo -e "\nRecoveryfs partition needs an upgrade."
  fi

  if [ "$recovery_upgrade_needed" == 1 ] || [ "$recoveryfs_upgrade_needed" == 1 ];
  then
    # Atleast one of the partitions needs an upgrade.
    # Extract the required images from the zipfile
    # and do a "thorough" sanity check on the them.
    ExtractAndCheckUpgradePackage $zipfile
  fi

  # Actually write into the partitions only if zip-sanity passes
  if [ "$recovery_upgrade_needed" == 1 ] && [ "$upgrade_zip_sanity_failed" == 0 ];
  then
    UpgradePartitionWithImage $recovery_partition $zipfile $recoveryimg
  fi

  if [ "$recoveryfs_upgrade_needed" == 1 ] && [ "$upgrade_zip_sanity_failed" == 0 ];
  then
    UpgradePartitionWithImage $recoveryfs_partition $zipfile $recoveryfsimg
  fi

  # Check basic sha1sum sanity of the upgraded partitions
  # Do this even if sanity of zip had failed. This is so that
  # if we have a remote chance that recovery and recoveryfs
  # partitions are actually fine, we do not deny future OTA upgrades.

  echo -e "\nPerforming basic sha1sum sanity checks on the upgraded partitions ..."

  if CompareSHA1Sum $recovery_partition $recoveryimg_length $recoveryimg_sha1;
  then
    echo -e "\nRecovery sanity passed ..."
  else
    echo -e "\nRecovery did not pass sanity ..."
    CheckExitCodeAndTerminate 1 0
  fi
  if CompareSHA1Sum $recoveryfs_partition $recoveryfsimg_length $recoveryfsimg_sha1;
  then
    echo -e "\nRecoveryfs sanity passed ..."
  else
    echo -e "\nRecoveryfs did not pass sanity ..."
    CheckExitCodeAndTerminate 1 0
  fi

  echo -e "\nUpgrade completed and sanity passed!!"
  WriteRecoveryUpgradeDoneCookie

fi
