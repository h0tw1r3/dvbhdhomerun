#!/bin/bash
SUBLEVEL=$(echo $1 | sed 's/[0-9]*\.[0-9]*\.*//' | sed 's/-.*//' | awk '{ printf "%s", $0 }')

KERNEL_VERSION=$(uname -r | sed "s/-.*//")

DIRNAME="linux-source-${KERNEL_VERSION}"

THEDIR=`pwd`
cd /usr/src/$DIRNAME
CONFNAME=$(echo "/lib/modules/${KERNEL_VERSION}/build/.config")
cp $CONFNAME .config

yes "" | make oldconfig

MSYMNAME=$(echo "/lib/modules/$(uname -r)/build/Module.symvers")
cp $MSYMNAME .
make prepare scripts

cd $THEDIR
sed -i "/^KERNEL_DIR/c\KERNEL_DIR      := /usr/src/$DIRNAME" Makefile
make KERNEL_VERSION=$1

cd /usr/src
rm -rf $DIRNAME
cd $THEDIR
