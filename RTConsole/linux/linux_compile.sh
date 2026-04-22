#April 22 2026: I had to make the build directory in the actual workspace root for vsc ~muodo

rm ../build
mkdir ../build
cd ../build
cmake -DDEFINE_RELEASE=ON ../linux
make -j 4
echo Copying binaries to ../bin directory, run from there!

cd ..