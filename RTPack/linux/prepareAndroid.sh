#!/bin/bash

usage=$(
cat <<EOF
Prepares a Proton Android project under Linux. Preprocesses any needed source files.
Execute this script in the directory where the project's AndroidManifest.xml file resides.

Usage: `basename $0` [options]

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

SHARED_ANDROID_DIR="../../shared/android"

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
dos2unix $ANDROID_MANIFEST

PACKAGE_NAME=$(awk -f $NDK_DIR/build/awk/extract-package-name.awk $ANDROID_MANIFEST)
PACKAGE_DIR=$(echo $PACKAGE_NAME | sed -e 's/\./\//g')
PACKAGE_NAME_WITH_UNDERSCORES=$(echo $PACKAGE_NAME | sed -e 's/\./_/g')
SMALL_PACKAGE_NAME=$(echo `grep LOCAL_MODULE jni/Android.mk | cut -d '=' -f 2`)

mkdir -p $TEMP_JAVA_SRC_DIR/$PACKAGE_DIR

# Copy app specific and shared java files to be pre-processed
rsync -v --update --delete --delete-excluded --recursive --exclude=.svn src/ $SHARED_ANDROID_DIR/v2_src/java/ $TEMP_JAVA_SRC_DIR/$PACKAGE_DIR

# Copy any extra libraries we need over - skip the preprocessing step for these, move them directly to the final dir

mkdir -p $FINAL_JAVA_SRC_DIR/com

# For IAP (optional)
if [[ "x$INCLUDE_IAP" == "xyes" ]];
then
	rsync -v --update --delete --delete-excluded --recursive --exclude=.svn $SHARED_ANDROID_DIR/optional_src/com/android $FINAL_JAVA_SRC_DIR/com/
fi

# For tapjoy (optional)
if [[ "x$INCLUDE_TAPJOY" == "xyes" ]];
then
	rsync -v --update --delete --delete-excluded --recursive --exclude=.svn $SHARED_ANDROID_DIR/optional_src/com/tapjoy $FINAL_JAVA_SRC_DIR/com/
fi

ANT_PROPERTIES="-DPACKAGE_NAME=$PACKAGE_NAME "
ANT_PROPERTIES+="-DSMALL_PACKAGE_NAME=$SMALL_PACKAGE_NAME "
ANT_PROPERTIES+="-DPACKAGE_NAME_WITH_UNDERSCORES=$PACKAGE_NAME_WITH_UNDERSCORES "

# Preprocess C++ sources
ant $ANT_PROPERTIES preprocess_cpp

# Preprocess Java sources
ant $ANT_PROPERTIES preprocess
