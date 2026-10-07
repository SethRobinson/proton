#!/bin/sh
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 1
[ -n "$script_dir" ] && [ "$script_dir" != / ] || exit 1
exec bash "$script_dir/../../RTPack/linux/update_media.sh" "$script_dir" "$@"
