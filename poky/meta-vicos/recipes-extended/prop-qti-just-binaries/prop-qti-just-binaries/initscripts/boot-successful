#!/bin/sh
set -e

SLOT_SUFFIX=`tr ' ' '\n' < /proc/cmdline | awk -F= /androidboot.slot_suffix/'{print $2}' | xargs`

case "$SLOT_SUFFIX" in
  '_a')
    THIS_SLOT='a'
    ;;
  '_b')
    THIS_SLOT='b'
    ;;
  *)
    THIS_SLOT='f'
esac

bootctl $THIS_SLOT mark_successful
setprop ro.boot.successful 1

dmesg > /data/boot.log
