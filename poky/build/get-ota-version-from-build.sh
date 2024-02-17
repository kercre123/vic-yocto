#!/usr/bin/env bash

set -e
set -u

SCRIPT_PATH=$(dirname $([ -L $0 ] && echo "$(dirname $0)/$(readlink -n $0)" || echo $0))

BUILD_ETC=${SCRIPT_PATH}/tmp-glibc/work/apq8009_robot-oe-linux-gnueabi/machine-robot-image/1.0-r0/rootfs/etc

BASE=`cat ${BUILD_ETC}/os-version-base`
CODE=`cat ${BUILD_ETC}/os-version-code`
OTA_VERSION="${BASE}.${CODE}"

echo $OTA_VERSION
