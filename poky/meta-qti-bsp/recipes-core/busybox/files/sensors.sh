#!/bin/sh
# Copyright (c) 2018, The Linux Foundation. All rights reserved.
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


MDEV=`echo $MDEV | sed 's#input/event##g'`

sensor_name=$(cat /sys/class/input/input$MDEV/name)

if [ $sensor_name  = smi130_accbuf ] || [ $sensor_name  = bmi160_accbuf ] || [ $sensor_name  = inv_accbuf ] || [ $sensor_name  = asm_accbuf ]; then
        ln -sf /dev/input/event$MDEV /dev/input/accbuff

elif [ $sensor_name  = smi130_gyrobuf ] || [ $sensor_name  = bmi160_gyrobuf ] || [ $sensor_name  = inv_gyrobuf ] || [ $sensor_name  = asm_gyrobuf ]; then
        ln -sf /dev/input/event$MDEV /dev/input/gyrobuff

elif [ $sensor_name = smi130_acc ] || [ $sensor_name = smi130_gyro ]; then
        chown -R root:sensors /sys/class/input/input$MDEV/*
        chmod 664  /sys/class/input/input$MDEV/*
fi
