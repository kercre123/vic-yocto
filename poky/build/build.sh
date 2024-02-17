#!/bin/bash

set -e

SCRIPT_NAME=$(basename $0)

# defaults
INCREMENTAL=0
VERBOSE=0
BUILD_COMMAND=("build-victor-robot-image")

function usage() {
    echo "$SCRIPT_NAME [OPTIONS] <bitbake build command/function> (default: ${BUILD_COMMAND[*]})"
    echo "  -h          print this message"
    echo "  -v          print verbose output"
}

while getopts ":hv" opt; do
    case $opt in
        h)
            usage
            exit 1
            ;;
        v)
            VERBOSE=1
            ;;
        :)
            echo "Options -${OPTARG} requires an argument." >&2
            usage
            exit 1
            ;;
    esac
done

if [ $VERBOSE -eq 1 ]; then
    set -x
fi

# Move past getops args
shift $(($OPTIND - 1))

if [ $# -gt 0 ]; then
    BUILD_COMMAND=$*
fi

case "${BUILD_COMMAND[0]}" in
    *incremental)
	INCREMENTAL=1
	;;
    *)
	INCREMENTAL=0
	;;
esac

echo "Build starting at `date`"

SCRIPT_PATH=$(dirname $([ -L $0 ] && echo "$(dirname $0)/$(readlink -n $0)" || echo $0))
TOPLEVEL=$(cd "${SCRIPT_PATH}/../.." && pwd)

# If we are not building the same git commit SHA as last time, do a full rebuild
OS_VER_REV_FILE="$TOPLEVEL/poky/build/os-version-rev"
if [ $INCREMENTAL -eq 1 ]; then
  if [ ! -f "${OS_VER_REV_FILE}" -o "`git rev-parse --short HEAD`" != "`cat $OS_VER_REV_FILE`" ]; then
      INCREMENTAL=0
  fi
fi

rm -rf ${OS_VER_REV_FILE}
# Remove build images, OTA files, etc.
git clean -dffx $TOPLEVEL/_build

# If we are doing a full build, clean everything, except the downloads folder
if [ $INCREMENTAL -eq 0 ]; then
    pushd $TOPLEVEL
    git clean -dffx -e poky/build/downloads .
    git submodule foreach --recursive 'git clean -dffx .'
    popd
fi

pushd $TOPLEVEL/poky

# Disable check for unset variables (it will make the bitbake scripts exit)
set +u
source build/conf/set_bb_env.sh

: ${ANKI_BUILD_VERSION:=0}
export ANKI_BUILD_VERSION
export BB_ENV_EXTRAWHITE="$BB_ENV_EXTRAWHITE ANKI_BUILD_VERSION"

${BUILD_COMMAND[*]}

popd

# Re-enable check for unset variables
set -u

git rev-parse --short HEAD > ${OS_VER_REV_FILE}
echo "Build finished at `date`"
