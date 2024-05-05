#! /bin/sh
#
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




check_if_configured()
{
                ret_val=`cat /sys/class/android_usb/android0/state | grep "CONFIGURED" |wc -l`
                echo $ret_val
}

check_if_disconnected()
{
                ret_val=`cat /sys/class/android_usb/android0/state | grep "DISCONNECTED" |wc -l`
                echo $ret_val
}

check_if_started()
{
                ret_val=`ps -fe | grep "mtpserver" | grep -v grep |wc -l`
                echo $ret_val
}


ret_val=`cat /sys/class/android_usb/android0/functions | grep "mtp" |wc -l`
if [ $ret_val -eq 1 ]; then

        result=`check_if_configured`
        result2=`check_if_started`
        if [ $result -eq 1 ] && [ $result2 -eq 0 ]; then
                echo -n "Starting mtpserver: " > /dev/kmsg
                /usr/bin/mtpserver &
                echo "done" > /dev/kmsg
        fi

        result=`check_if_disconnected`
        if [ $result -eq 1 ]; then
            echo -n "Stoping mtpserver: " > /dev/kmsg
            killall mtpserver
            echo "done" > /dev/kmsg
        fi
fi

