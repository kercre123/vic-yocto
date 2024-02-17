#!/usr/bin/env bash

set -e
set -u

VERBOSE=0
PRE=""

function usage() {
    echo "$SCRIPT_NAME [OPTIONS] tags build with ota version and/or prepend value"
    echo "  -h          print this message"
    echo "  -v          print verbose output"
    echo "  -p [STRING] prepend STRING to tag"
}

while getopts "hvp:" opt; do
    case $opt in
        h)
            usage
            exit 1
            ;;
        v)
            VERBOSE=1
            ;;
        p)
            PRE="${OPTARG}-"
            ;;
   esac
done

if [ $VERBOSE -eq 1 ]; then
    set -x
fi

SCRIPT_PATH=$(dirname $([ -L $0 ] && echo "$(dirname $0)/$(readlink -n $0)" || echo $0))
TAGNAME=${PRE}`${SCRIPT_PATH}/get-ota-version-from-build.sh`

# Check to see if the victor submodule pointer has been modified
VICTOR_DIR=anki/victor
ANKI_VICTOR_STATUS=`git status --porcelain $VICTOR_DIR | awk '{print $1;}'`

if [ "$ANKI_VICTOR_STATUS" = "M" ]; then
    pushd $VICTOR_DIR
    # Get the SHA for the Victor checkout
    VICTOR_SHA=`git log -1 --format=%h`
    popd

    # Prepare a commit message for updating the victor submodule pointer
    echo "[$VICTOR_DIR $VICTOR_SHA] Build $TAGNAME" > msg.txt
    echo "" >> msg.txt
    git submodule summary >> msg.txt

    # Make a commit for updating the victor submodule pointer
    git add $VICTOR_DIR
    git commit -F msg.txt
    rm msg.txt
fi

# Create a tag and push it to the remote
git tag ${TAGNAME} HEAD

git push origin ${TAGNAME}:refs/tags/${TAGNAME}
