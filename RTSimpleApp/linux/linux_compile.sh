rm build/RTSimpleApp
mkdir build
cd build
cmake -DDEFINE_RELEASE=ON ..
make -j 4
cd ..
