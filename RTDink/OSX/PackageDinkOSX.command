#!/bin/bash



BUILT_PRODUCTS_DIR=/Users/Seth/build/Build/Products/Release
PROJECT_DIR=/Volumes/projects/proton/RTDink/OSX
#Note: It assumes your .app is in ~/Build/Release which may not be the case…

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

set -ex

PROJECT_NAME="Dink Smallwood HD"
TEMP_FILES_DIR=~/TEMP
dir=$TEMP_FILES_DIR/disk
echo “Temp dir is $TEMP_FILES_DIR ”

dmg="$BUILT_PRODUCTS_DIR/$PROJECT_NAME.dmg"
touch ~/"$PROJECT_NAME.dmg"

rm ~/"$PROJECT_NAME.dmg"
rm -rf "$dir"
echo Creating $dir

mkdir "$dir"
cp -R "$BUILT_PRODUCTS_DIR/$PROJECT_NAME.app" "$dir"
cp "$PROJECT_DIR/readme.txt" "$dir"
rm -f "$dmg"
echo Creating $dir

hdiutil create -srcfolder "$dir" -volname "$PROJECT_NAME" "$dmg"
hdiutil internet-enable -yes "$dmg"

rm -rf "$dir"
cp "$dmg" ~/

cp "$dmg" "$PROJECT_DIR/../script"

Pause

