rm ./rtsimpleapp
cmake -DDEFINE_RELEASE=ON ./
make -j 4
cp ./rtsimpleapp ../bin

