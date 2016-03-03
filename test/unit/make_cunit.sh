#!/bin/bash

# utility functions for doing configure and make

check_error()
{
  RC=$1
  MSG=$2
  FUN=$3
  if [ $RC -ne 0 ]; then
    echo "$2"
    ($FUN)
    exit $RC
  fi
}

# $1 = the directory where the source code is
# $2 = the directory where the build is going to take place
configure_build()
{
  SRC_DIR=$1
  BUILD_DIR=$2
  RC=0

  cd $SRC_DIR
  check_error $? "$SRC_DIR does not exist."

  # specific build steps pulled from stackoverflow : https://stackoverflow.com/questions/12514408/building-cunit-on-windows
  libtoolize
  check_error $? "libtoolize could not be run in $SRC_DIR"

  automake --add-missing

  autoreconf
  check_error $? "autoreconf could not be run in $SRC_DIR"

  mkdir -p $BUILD_DIR
  check_error $? "Could not create $BUILD_DIR"

  ./configure --prefix=$BUILD_DIR
  RC=$?

  cd -
  return $?
}

# $1 = the directory where the code is going to be built 
make_build()
{
  BUILD_DIR=$1

  cd $BUILD_DIR
  check_error $? "$BUILD_DIR does not exist"

  echo "Building in $BUILD_DIR"

  make clean
  check_error $? "make clean failed"
  make all
  check_error $? "make all failed"
  make install
  check_error $? "make install failed"

  RC=$?
  cd -
  return $?
}
