#April 22 2026: build directory is one level up (in RTConsole/build) so VSCode/clangd can pick up the exported compile_commands.json from the workspace root. ~muodo

rm -rf ../build
mkdir -p ../build
cd ../build
cmake -DDEFINE_RELEASE=ON ../linux
make -j 4
echo Build complete. Binary is in ../bin/RTConsole - run from there!
cd ..
