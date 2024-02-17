# Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted (subject to the limitations in the
# disclaimer below) provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
#     * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
# GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
# HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# This scripts is used to filter out not necessary packages
# from initramfs root file system.

set_devices () {
  [ -e "dev/null" ]    || mknod dev/null c 1 3
  [ -e "dev/zero" ]    || mknod dev/zero c 1 5
  [ -e "dev/urandom" ] || mknod dev/urandom c 1 9
  [ -e "dev/console" ] || mknod dev/console c 5 1
}

add_folder () {
  if ! [ -e $1 ]; then
    mkdir -p $1
  fi
}

remove_file () {
  if ! [ -d $1 ]; then
      rm -rf $1
  fi
}

remove_all () {
    rm -rf "$1"
}

configure_final_image () {
  cd ${IMAGE_ROOTFS}

  # To speed up the build, delete the known path directory
  known_list="
            ./lib/udev
            ./lib/systemd
            ./etc/udev
            ./usr/lib/opkg
            ./usr/share
            ./usr/bin/update-alternatives
            ./usr/libexec/udevadm
            ./bin/udevadm
            ./sbin/ldconfig
            ./sbin/udevd
            ./etc/lvm
            ./usr/sbin/dmeventd
            ./usr/sbin/lv*
            ./usr/sbin/pv*
            ./usr/lib/liblvm2*
            ./usr/lib/tmpfiles*
            ./usr/lib/libaio*
            ./usr/sbin/vg*
            "

  if ${@bb.utils.contains('DISTRO_FEATURES','nad-fde','true','false',d)}; then
    known_list_km="
            ./usr/lib/libcontent*
            ./usr/lib/libcpion*
            ./usr/lib/libhdcpsrm*
            ./usr/lib/liblmclient*
            ./usr/lib/libqcbor*
            ./usr/lib/libqsappsver*
            ./usr/lib/libqwes*
            ./usr/lib/libtzcom*
            ./usr/lib/gio
            ./usr/lib/libbz2.so.1*
            ./usr/lib/libconfigdb.so.0*
            ./usr/lib/libcrypt.so.1*
            ./usr/lib/libcurl.so.4*
            ./usr/lib/libcv2x_sysctrl.so.0*
            ./usr/lib/libdbus-1.so.3*
            ./usr/lib/libdiag.so.1*
            ./usr/lib/libdsi_netctrl.so.1*
            ./usr/lib/libdsutils.so.1*
            ./usr/lib/libexpat.so.1*
            ./usr/lib/libffi.so.6*
            ./usr/lib/libgio-2.0.so.0*
            ./usr/lib/libglib-2.0.so.0*
            ./usr/lib/libgmodule-2.0.so.0*
            ./usr/lib/libgmp.so.10*
            ./usr/lib/libgnutls.so.30*
            ./usr/lib/libgobject-2.0.so.0*
            ./usr/lib/libgthread-2.0.so.0*
            ./usr/lib/libhogweed.so.4*
            ./usr/lib/libidn2.so.0*
            ./usr/lib/liblzma.so.5*
            ./usr/lib/libminksocket.so*
            ./usr/lib/libnetmgr_common.so.0*
            ./usr/lib/libnetmgr.so.0*
            ./usr/lib/libnettle.so.6*
            ./usr/lib/libowcrypt.so.1
            ./usr/lib/libpowermanager.so.1*
            ./usr/lib/libpugixml.so.1*
            ./usr/lib/libqcmap*
            ./usr/lib/libqdi.so.0*
            ./usr/lib/libqdp.so.0*
            ./usr/lib/libqmi*
            ./usr/lib/librmnetctl.so.0*
            ./usr/lib/libsemanage.so.1
            ./usr/lib/libtime_genoff.so.1*
            ./usr/lib/libunistring.so.2*
            ./usr/lib/libutils.so.0*
            ./usr/lib/libxml*
            ./usr/lib/pkgconfig
            ./usr/lib/ssl-1.1
            ./usr/lib/systemd
            ./usr/lib/sysusers.d
            ./usr/lib/tmpfiles.d
            ./usr/bin/hdcp*
            ./usr/bin/km_lite_sample_client
            ./usr/bin/qmi_client
            ./usr/bin/qwes*
            ./usr/bin/seemp_healthd
            ./usr/bin/prdrmkeyprov
            ./usr/bin/InstallKeybox
            ./usr/bin/StoreKeybox
            ./usr/bin/chfnadpl
            ./usr/bin/chage
            ./usr/bin/chfn*
            ./usr/bin/chsh*
            ./usr/bin/dbus-*
            ./usr/bin/ethpwrmgr
            ./usr/bin/expiry
            ./usr/bin/faillog
            ./usr/bin/gpasswd
            ./usr/bin/ipacmdiag
            ./usr/bin/lastlog
            ./usr/bin/netmgrd
            ./usr/bin/nettle-*
            ./usr/bin/newgidmap
            ./usr/bin/newgrp*
            ./usr/bin/newuidmap
            ./usr/bin/pkcs1-conv
            ./usr/bin/port_bridge
            ./usr/bin/QCMAP_*
            ./usr/bin/qmi_test*
            ./usr/bin/qmuxbridge
            ./usr/bin/qti*
            ./usr/bin/radish
            ./usr/bin/sexp-conv
            ./usr/bin/sg
            ./usr/bin/qseecom_sample_client
            "
  fi

  for file in ${known_list}; do
      remove_all "${file}"
  done

  if ${@bb.utils.contains('DISTRO_FEATURES','nad-fde','true','false',d)}; then
    for file in ${known_list_km}; do
        remove_all "${file}"
    done
  fi

  for file in $(find); do
    if echo "$file" | grep -E "/bin|usr/lib|./sbin" > /dev/null; then
      continue
    elif echo "$file" | grep "./usr/sbin" > /dev/null; then
      case $file in
        */cryptsetup) ;;
        */dmsetup) ;;
        */ubi*) ;;
        *) remove_file $file
          continue ;;
      esac
    elif echo "$file" | grep "./lib" > /dev/null; then
      case $file in
        */ld-*) ;;
        */libc*) ;;
        */libdl*) ;;
        */librt*) ;;
        */libpthread*) ;;
        */libuuid*) ;;
        */libdev*) ;;
        */libudev*) ;;
        */libcypt*) ;;
        */libz.so*) ;;
        */libm*) ;;
        */libresolv*) ;;
        */libblkid*) ;;
        */libmount*) ;;
        */libpcre*) ;;
        */libselinux*) ;;
        */libssl*) ;;
        */libgcc_s*) ;;
        */firmware) ;;
        *) remove_file $file
          continue ;;
      esac
    else
      case $file in
        ./init) ;;
        ./dev/console) ;;
        ./dev/tty*) ;;
        ./dev/null) ;;
        ./dev/urandom) ;;
        ./dev/zero) ;;
        ./etc/keys*) ;;
        *) remove_file $file
          continue ;;
      esac
    fi
  done

  # To make the boot up faster here prepare the device instead of
  # create them during boot up
  for folder in dev mnt proc run sys tmp var; do
    add_folder "${folder}"
  done

  if ${@bb.utils.contains('DISTRO_FEATURES','nad-fde','true','false',d)}; then
    ln -sf /firmware/image ./lib/firmware/updates
  fi
  set_devices
}

