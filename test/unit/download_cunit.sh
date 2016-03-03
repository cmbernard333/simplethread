#!/bin/sh

# source the functions here
. ./make_cunit.sh

CURL=$(which curl)
WGET=$(which wget)
URL="http://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fcunit%2Ffiles%2FCUnit%2F2.1-3%2F&ts=1457026651&use_mirror=iweb"
USE_WGET=0
CUNIT="CUnit-2.1-3.tar.bz2"
CUNIT_DIR=$(basename $CUNIT .tar.bz2)

if [ -z "$CURL" ]; then
  echo "Couln't find curl. Trying wget..."
  USE_WGET=1
fi

if [ $USE_WGET -eq 1 -a -z "$WGET" ]; then
  echo "Couldn't find wget. Can't download..."
  exit 1
fi

if [ ! -e $CUNIT ]; then
  if [ $USE_WGET -eq 1 ]; then
    wget $URL
  else
    curl -L $URL -o $CUNIT
  fi
else
  echo "Already have $CUNIT. Proceeding to build.."
fi

# check download
if [ $? -ne 0 ]; then
  echo "Couldn't download $CUNIT. Try again later..."
  exit 1
fi

# decompress it
if [ -e "$CUNIT" ]; then
  if [ -e "$CUNIT_DIR" ]; then
    rm -rf $CUNIT_DIR
  fi
  tar -jxf "$CUNIT"
else
  echo "Download for $CUNIT was successful, but can't find it."
  exit 1
fi

echo "Download of $CUNIT completed successfully."

# build everything
configure_build $CUNIT_DIR $(pwd)/$CUNIT_DIR/build-$CUNIT_DIR
check_error $? "Configure failed."
make_build $CUNIT_DIR
check_error $? "Make failed."

exit $?
