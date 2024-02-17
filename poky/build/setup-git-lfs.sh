#!/usr/bin/env bash

set -e
set -u

SCRIPT_PATH=$(dirname $([ -L $0 ] && echo "$(dirname $0)/$(readlink -n $0)" || echo $0))

# Verify that git exists
which git || (echo "git does not exist in your PATH" && exit 1)

cd ${SCRIPT_PATH}/../../anki/victor

git lfs install
