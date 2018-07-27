rm build/RTPack
mkdir build
cd build
cmake -DDEFINE_RELEASE=ON ..
make -j 4
echo Copying binaries to ../bin directory, run from there!
cp RTPack ../../bin
cd ..
