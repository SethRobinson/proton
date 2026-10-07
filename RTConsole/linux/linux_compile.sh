#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 1
[ -n "$script_dir" ] && [ "$script_dir" != / ] || exit 1
app_dir=$(CDPATH= cd -- "$script_dir/.." && pwd -P) || exit 1
. "$script_dir/../../shared/linux/safe_paths.sh"
#April 22 2026: build directory is one level up (in RTConsole/build) so VSCode/clangd can pick up the exported compile_commands.json from the workspace root. ~muodo

proton_remove_tree "$app_dir" build || exit 1
mkdir -p "$app_dir/build"
cd -- "$app_dir/build" || exit 1
cmake -DDEFINE_RELEASE=ON ../linux
make -j 4
echo Build complete. Binary is in ../bin/RTConsole - run from there!
cd ..
