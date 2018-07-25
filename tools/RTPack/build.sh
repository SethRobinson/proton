#!/bin/sh

# A script for building RTPack on Linux

# Go to the directory of the script
START_DIR=`pwd`
cd `dirname $0`

cleanexit() {
	# Return to the directory where we started from
	cd $START_DIR
	exit $1
}


mkdir -p build
cd build

if ! cmake ..; then cleanexit 1; fi
if ! make; then cleanexit 1; fi

cleanexit 0
