#!/bin/bash

set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 1
[ -n "$script_dir" ] && [ "$script_dir" != / ] || exit 1
. "$script_dir/../../shared/linux/safe_paths.sh"
: "${1:?Usage: update_media.sh /absolute/project/media [options]}"
media_dir=$1
proton_child_path "$media_dir" interface >/dev/null || exit 1
[ -f "$media_dir/exclude.txt" ] || exit 1
app_dir=$(CDPATH= cd -- "$media_dir/.." && pwd -P) || exit 1
[ "$media_dir" = "$app_dir/media" ] || exit 1
proton_child_path "$app_dir" bin >/dev/null || exit 1
cd -- "$media_dir" || exit 1
shift
PACK_EXE="$script_dir/../bin/RTPack"

if [ ! -x "$PACK_EXE" ];
then
	echo "Missing RTPack tool (from ${PACK_EXE})"
	echo "Go into /RTPack/linux and run "sh compile_linux.sh" to build the tool first!"
	exit 1
fi

usage=$(
cat <<EOF
Usage: $0 /absolute/project/media [options]

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
		cd "$1" || exit 1
		TEXTURE_CONVERSION_OPTS=$(cat texture_conversion_flags.txt 2>/dev/null || true)
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
rm -f -- "${media_dir:?}/interface/"font_*.rttex

echo Copy the stuff we care about

copy_media_to_bin() {
	if [ -d "$1" ];
	then
		case "$1" in interface|audio|game) ;; *) exit 1;; esac
        proton_child_path "$app_dir" "bin/$1" >/dev/null || exit 1
        [ -f "$media_dir/$2" ] || exit 1
        sed -e 's/^\..*$/*&/g' "$media_dir/$2" | rsync -v --update --delete --delete-excluded --recursive --exclude-from=- -- "$media_dir/$1" "${app_dir:?}/bin/"
	fi
}

copy_media_to_bin interface exclude.txt
copy_media_to_bin audio exclude.txt
copy_media_to_bin game game_exclude.txt

rm -f -- "${media_dir:?}/icon.rttex"
rm -f -- "${media_dir:?}/default.rttex"
