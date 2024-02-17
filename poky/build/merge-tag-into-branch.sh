#!/bin/bash

set -e

SCRIPT_NAME=`basename ${0}`

function usage()
{
    echo "usage: $SCRIPT_NAME <branch name> <tag name>"
    echo " branch name              Like master, release/candidate, etc"
    echo " tag name                 Like 1.1.0.1890, 1.0.1.1865, etc"
    exit 1
}

if [ -z "$1" ]; then
    usage
fi

if [ -z "$2" ]; then
    usage
fi

set -u

BRANCH=$1
TAGNAME=$2

# Verify that git exists
which git || (echo "git does not exist in your PATH" && exit 1)

# Clean up
git clean -dffx .
git submodule foreach --recursive 'git clean -dffx .'

# Fetch, checkout, and update submodules
git fetch --tags
git checkout --quiet origin/${BRANCH}
git submodule update --init --recursive
git submodule update --recursive

# Merge in from tag and update submodules
git merge --no-edit ${TAGNAME}
git submodule update --init --recursive
git submodule update --recursive

# Push to ${BRANCH}
git push origin HEAD:refs/heads/${BRANCH}

