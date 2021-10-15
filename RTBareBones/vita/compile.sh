# Compile Script for vpk

#rm -rf build
#mkdir build
cd build
cmake ..
make -j8

#move vpk to bin
mv RTBareBones.vpk ../../bin/RTBareBones.vpk