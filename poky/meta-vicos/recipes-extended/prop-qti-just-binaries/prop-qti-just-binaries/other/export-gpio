#!/bin/bash

set -e
set -u

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 pin-number"
  echo "Example: $0 94"
  exit 1
fi

PIN=$1

[[ -e /sys/class/gpio/gpio${PIN} ]] || /bin/echo ${PIN} > /sys/class/gpio/export
