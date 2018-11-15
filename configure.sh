#!/bin/bash


BASEDIR=`pwd`
echo $BASEDIR
echo "Setting up for vaultdb"
echo -n Password:
read -s PASSWORD
#PASSWORD="" #password"

rm -rf build 
rm -rf lib
rm -rf external

mkdir build && mkdir lib && mkdir external && cd external # make build dir and lib dir
## IN LIB folder
echo $PASSWORD | sudo -S apt-get update
echo $PASSWORD | sudo -S apt-get -y install build-essential autoconf libtool pkg-config cmake git libgflags-dev libgoogle-perftools-dev postgresql-all


#grpc installation
git clone --branch v1.16.1 https://github.com/grpc/grpc.git
    cd grpc
    git submodule update --init
    make HAS_SYSTEM_PROTOBUF=false -j10
    make prefix=$BASEDIR/lib install
    cd third_party/protobuf
    make prefix=$BASEDIR/lib install
# back in external
cd $BASEDIR/external

#G3Log installation
git clone --branch 1.3.2 https://github.com/KjellKod/g3log.git
cd g3log
cd 3rdParty/gtest
unzip gtest-1.7.0.zip
cd ../../
mkdir build
cd build
cmake -DCPACK_PACKAGING_INSTALL_PREFIX=$BASEDIR/lib ..
make -j10
make install


#libpqxx installation
cd $BASEDIR/external
git clone --branch 6.2.5 https://github.com/jtv/libpqxx.git
cd libpqxx
./configure --prefix=$BASEDIR/lib/ --disable-documentation
make -j10
make install

cd $BASEDIR
cd build
cmake ..
make -j10 vaultdb
make -j10
ctest 



