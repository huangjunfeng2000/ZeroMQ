#!/usr/bin/env sh
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Please refer to the README for information about making permanent changes.  #
################################################################################
#
# This script extracts the version from the project header file
#
project=$1
appendix="_library"
if [ ! -f include/$project$appendix.h ]; then
    echo 2.0.0 | tr -d '\n'
    exit 0
fi
MAJOR=`egrep '^#define .*_VERSION_MAJOR +[0-9]+$' include/$project$appendix.h`
MINOR=`egrep '^#define .*_VERSION_MINOR +[0-9]+$' include/$project$appendix.h`
PATCH=`egrep '^#define .*_VERSION_PATCH +[0-9]+$' include/$project$appendix.h`
if [ -z "$MAJOR" -o -z "$MINOR" -o -z "$PATCH" ]; then
    echo "version.sh: error: could not extract version from include/$project$appendix.h" 1>&2
    exit 1
fi
MAJOR=`echo $MAJOR | awk '{ print $3 }'`
MINOR=`echo $MINOR | awk '{ print $3 }'`
PATCH=`echo $PATCH | awk '{ print $3 }'`
echo $MAJOR.$MINOR.$PATCH | tr -d '\n'

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Please refer to the README for information about making permanent changes.  #
################################################################################
