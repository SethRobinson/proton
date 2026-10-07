#!/bin/bash
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 1
. "$script_dir/../../shared/linux/safe_paths.sh"
: "${1:?Usage: androidSyncAssets.sh /absolute/project/android}"
project_dir=$1
[ -f "$project_dir/AndroidManifest.xml" ] || exit 1
proton_child_path "$project_dir" assets >/dev/null || exit 1
[ -d "$project_dir/../bin" ] || exit 1
rsync --recursive --perms --delete --delete-excluded --exclude=log.txt --exclude=save.dat '--exclude=*.mp3' -- "$project_dir/../bin/" "${project_dir:?}/assets/"
