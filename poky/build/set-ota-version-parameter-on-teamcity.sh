#!/usr/bin/env bash

set -u
set -e

SCRIPT_PATH=$(dirname $([ -L $0 ] && echo "$(dirname $0)/$(readlink -n $0)" || echo $0))
OTA_VERSION=`${SCRIPT_PATH}/get-ota-version-from-build.sh`

echo "##teamcity[setParameter name='OTA_VERSION' value='${OTA_VERSION}']"

