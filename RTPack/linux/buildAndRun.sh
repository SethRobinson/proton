#!/bin/bash
set -eu
TOOLDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 1
: "${1:?Usage: buildAndRun.sh /absolute/project/linux}"
case "$1" in /*) ;; *) exit 1;; esac
project_dir=$(CDPATH= cd -- "$1" && pwd -P) || exit 1
app_dir=$(CDPATH= cd -- "$project_dir/.." && pwd -P) || exit 1
cd -- "$project_dir" || exit 1

if [[ ! -f "CMakeLists.txt" ]];
then
	echo "No CMakeLists.txt found. Is this really a directory for a linux build?"
	exit 1
fi


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
cd -- "$app_dir/media" || exit 1
bash "$TOOLDIR/update_media.sh" "$app_dir/media"

# Run
cd -- "$app_dir/bin" || exit 1
"$EXECUTABLE"
