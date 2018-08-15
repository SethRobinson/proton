#!/bin/bash

PACK_EXE=$(cd `dirname $0`/../bin/; pwd)/RTPack

if [ ! -x ${PACK_EXE} ];
then
	echo "Missing RTPack tool (from ${PACK_EXE})"
	echo "Go into /RTPack/linux and run "sh compile_linux.sh" to build the tool first!"
	exit 1
fi

usage=$(
cat <<EOF
Usage: $0 [options]

Options:
-h               Print this help and exit
EOF
)

while getopts "h" OPTION; do
	case "$OPTION" in
		h)
			echo "$usage"
			exit 0
			;;
		*)
			echo "$usage"
			exit 1
			;;
	esac
done

echo Make fonts

find . -depth -name 'font*.txt' -exec ${PACK_EXE} -make_font '{}' ';'

echo Process our images and textures and copy them into the bin directory

process_directory_images() {
	
	echo Processing $1
	
	if [ -d "$1" ];
	then
		cd "$1"
		TEXTURE_CONVERSION_OPTS=$(cat texture_conversion_flags.txt)
		if [ -z "$TEXTURE_CONVERSION_OPTS" ]; then
			echo "Texture conversion flags are empty."
			echo "Hmm, $1 doesn't have a texture_conversion_flags.txt file in the dir, so using default parms of -8888"
			TEXTURE_CONVERSION_OPTS="-8888"
		fi
		
		for IMG in `find . -depth \( -name '*.jpg' -o -name '*.bmp' -o -name '*.png' \) -print`; do
			RTTEX=`echo $IMG | sed -e 's/\.\(jpg\|bmp\|png\)$/.rttex/'`
			#if [ "$IMG" -nt "$RTTEX" ]; then
				$PACK_EXE $TEXTURE_CONVERSION_OPTS "$IMG"
				$PACK_EXE "$RTTEX"
			#fi
		done 
		cd - > /dev/null
	fi
}

process_directory_images game
process_directory_images interface

echo Delete things we do not want copied
rm -f interface/font_*.rttex

echo Copy the stuff we care about

copy_media_to_bin() {
	if [ -d "$1" ];
	then
		sed -e 's/^\..*$/*&/g' "$2" | rsync -v --update --delete --delete-excluded --recursive --exclude-from=- "$1" ../bin
	fi
}

copy_media_to_bin interface exclude.txt
copy_media_to_bin audio exclude.txt
copy_media_to_bin game game_exclude.txt

rm -f icon.rttex
rm -f default.rttex
