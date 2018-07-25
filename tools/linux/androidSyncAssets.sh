#!/bin/bash


ANDROID_MANIFEST="AndroidManifest.xml"
if [[ ! -f "$ANDROID_MANIFEST" ]];
then
	echo "No $ANDROID_MANIFEST found in the current directory. Is this really a directory for an Android build?"
	exit 1
fi

SOURCE_DIR="../bin/"
TARGET_DIR="assets"

rsync --recursive --perms --delete --delete-excluded --exclude=log.txt --exclude=save.dat '--exclude=*.mp3' "$SOURCE_DIR" "$TARGET_DIR"
