#!/bin/bash
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 1
[ -n "$script_dir" ] && [ "$script_dir" != / ] || exit 1
. "$script_dir/../../shared/linux/safe_paths.sh"
: "${1:?Usage: prepareAndroid.sh /absolute/project/android [options]}"
project_dir=$1
proton_child_path "$project_dir" temp_src >/dev/null || exit 1
proton_child_path "$project_dir" temp_final_src >/dev/null || exit 1
cd -- "$project_dir" || exit 1
shift

usage=$(
cat <<EOF
Prepares a Proton Android project under Linux. Preprocesses any needed source files.
Pass the absolute directory containing the project's AndroidManifest.xml.

Usage: `basename $0` /absolute/project/android [options]

Options:
-h               Print this help and exit

-i               Include IAP sources (default is off)
-t               Include Tapjoy sources (default is off)
EOF
)

while getopts "hit" OPTION; do
	case "$OPTION" in
		h)
			echo "$usage"
			exit 0
			;;
		i)
			INCLUDE_IAP="yes"
			;;
		t)
			INCLUDE_TAPJOY="yes"
			;;
		*)
			echo "$usage"
			exit 1
			;;
	esac
done

ANDROID_MANIFEST="AndroidManifest.xml"
TEMP_JAVA_SRC_DIR="temp_src"
FINAL_JAVA_SRC_DIR="temp_final_src"

SHARED_ANDROID_DIR="$script_dir/../../shared/android"

if [[ ! -f "$ANDROID_MANIFEST" ]];
then
	echo "No $ANDROID_MANIFEST found in the current directory. Is this really a directory for an Android build?"
	exit 1
fi

NDK_DIR=$(dirname `which ndk-build 2> /dev/null` 2> /dev/null)
if [[ ! -d "$NDK_DIR" ]];
then
	echo "Couldn't find Android NDK. Make sure ndk-build is in \$PATH."
	exit 1
fi

# Make sure there are unix line ends in the AndroidManifest.xml. Otherwise awk might get confused.
dos2unix "$ANDROID_MANIFEST"

PACKAGE_NAME=$(awk -f "$NDK_DIR/build/awk/extract-package-name.awk" "$ANDROID_MANIFEST")
case "$PACKAGE_NAME" in ''|.*|*.|*..*|*[!a-zA-Z0-9_.]*) echo 'Invalid package name' >&2; exit 1;; esac
PACKAGE_DIR=$(echo $PACKAGE_NAME | sed -e 's/\./\//g')
PACKAGE_NAME_WITH_UNDERSCORES=$(echo $PACKAGE_NAME | sed -e 's/\./_/g')
SMALL_PACKAGE_NAME=$(echo `grep LOCAL_MODULE jni/Android.mk | cut -d '=' -f 2`)

proton_child_path "$project_dir" "temp_src/$PACKAGE_DIR" >/dev/null || exit 1
mkdir -p "$project_dir/temp_src/$PACKAGE_DIR"

# Copy app specific and shared java files to be pre-processed
rsync -v --update --delete --delete-excluded --recursive --exclude=.svn -- "$project_dir/src/" "$SHARED_ANDROID_DIR/v2_src/java/" "${project_dir:?}/temp_src/${PACKAGE_DIR:?}/"

# Copy any extra libraries we need over - skip the preprocessing step for these, move them directly to the final dir

proton_child_path "$project_dir" temp_final_src/com >/dev/null || exit 1
mkdir -p "$project_dir/temp_final_src/com"

# For IAP (optional)
if [[ "x${INCLUDE_IAP-}" == "xyes" ]];
then
	rsync -v --update --delete --delete-excluded --recursive --exclude=.svn -- "$SHARED_ANDROID_DIR/optional_src/com/android" "${project_dir:?}/temp_final_src/com/"
fi

# For tapjoy (optional)
if [[ "x${INCLUDE_TAPJOY-}" == "xyes" ]];
then
	rsync -v --update --delete --delete-excluded --recursive --exclude=.svn -- "$SHARED_ANDROID_DIR/optional_src/com/tapjoy" "${project_dir:?}/temp_final_src/com/"
fi

ANT_PROPERTIES="-DPACKAGE_NAME=$PACKAGE_NAME "
ANT_PROPERTIES+="-DSMALL_PACKAGE_NAME=$SMALL_PACKAGE_NAME "
ANT_PROPERTIES+="-DPACKAGE_NAME_WITH_UNDERSCORES=$PACKAGE_NAME_WITH_UNDERSCORES "

# Preprocess C++ sources
ant $ANT_PROPERTIES preprocess_cpp

# Preprocess Java sources
ant $ANT_PROPERTIES preprocess
