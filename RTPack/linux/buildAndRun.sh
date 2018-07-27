#!/bin/bash

if [[ ! -f "CMakeLists.txt" ]];
then
	echo "No CMakeLists.txt found. Is this really a directory for a linux build?"
	exit 1
fi

TOOLDIR=$(cd `dirname $0`; pwd)

# Build
mkdir -p build
cd build

if ! cmake ..; then exit 1; fi
if ! make; then exit 1; fi

# Try to figure out the executable name
if [[ `find . -maxdepth 1 -type f -executable | wc -l` -eq "1" ]]; then
	EXECUTABLE=`pwd`/$(find . -maxdepth 1 -type f -executable)
else
	echo "Can't figure out the executable name. Run the application manually."
	exit 1
fi

# Update media
cd ../../media
$TOOLDIR/update_media.sh

# Run
cd ../bin
$EXECUTABLE
