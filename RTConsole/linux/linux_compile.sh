#Muodo's TODO before PR - Implement linux compilations.

rm build/RTConsole
mkdir build
cd build
cmake -DDEFINE_RELEASE=ON ..
make -j 4
echo Copying binaries to ../bin directory, run from there!


#21st of April 2026: updated CMakeLists to put bins to ../bin by themselves
#cp RTConsole ../../bin

cd ..
