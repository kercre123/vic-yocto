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
# trigger-recovery-updater
# This is an init script that spawns recovery-updater.sh as a daemon.
# The daemon will be launched as user-system:group-system.
# an upgrade.
# This script itself runs as root.
#

set -e

PIDFILE=/var/run/recovery-updater.pid

case "$1" in
  start)
        # chgrp of /cache to "system" so that
        # recovery-updater can also access it
        find /cache -exec chgrp system {} +
        chmod -R g+rw /cache
        find /cache -type d -exec chmod g+s {} +
        echo -e "Starting recovery-updater ..." > /dev/kmsg

        # start recovery-updater as user:system and group:system
        start-stop-daemon -S -m -p $PIDFILE \
                -c system:system -b -a /etc/recovery-updater.sh

        pid=`cat $PIDFILE`
        echo -e "\nStarted recovery-updater! pid - $pid ..." > /dev/kmsg
        rm -f $PIDFILE #clean up the pid file to avoid reading stale entries
        ;;
  stop)
        # Do nothing
        ;;
  restart)
        # Do nothing
        ;;
  *)
        echo "Usage $0 { start | stop | restart}" >&2
        exit 1
        ;;
esac

exit 0
