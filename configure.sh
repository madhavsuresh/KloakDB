#!/bin/bash

echo "Setting up for vaultdb"
PASSWORD="password"

mkdir build && mkdir lib && cd lib # make build dir and lib dir
## IN LIB folder
echo $(PASSWORD) | sudo -S apt-get update
echo $(PASSWORD) | sudo -S apt-get -y install build-essential autoconf libtool pkg-config cmake git libprotoc-dev libgflags-dev

git clone https://github.com/grpc/grpc.git
    # in grpc
    cd grpc
    git submodule update --init
    make
    echo password | sudo -S make install
# back in lib
cd ..

#G3Log installation
git clone https://github.com/KjellKod/g3log.git
cd g3log
cd 3rdParty/gtest
unzip gtest-1.7.0.zip
cd ../../
mkdir build
cd build
cmake ..
make
echo $(PASSWORD) | sudo -S make install


cd .. # back to /lib
echo password | sudo -S apt-get -y install libpqxx-dev # get libpqxx
echo $(PASSWORD) | sudo -S apt-get -y install protobuf-compiler-grpc libgoogle-perftools-dev

cd ~/vaultdb_operators # back to root vaultdb
cd build
cmake ..
make vaultdb



