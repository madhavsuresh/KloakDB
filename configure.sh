#!/bin/bash

BASEDIR=`pwd`
echo $BASEDIR
echo "Setting up for vaultdb"

if [ ! -d "build" ]; then
	mkdir build
fi

if [ ! -d "external" ]; then
	mkdir external
fi

if [ ! -d "lib" ]; then
	mkdir lib
fi



#gflags installation
cd $BASEDIR/external
if [ ! -d "gflags" ]; then
	git clone https://github.com/gflags/gflags.git
	cd gflags
	mkdir build && cd build
	cmake -DCMAKE_INSTALL_PREFIX=$BASEDIR/lib ..
	make -j10
	make install
	cd $BASEDIR/external
fi

#grpc installation
cd $BASEDIR/external
if [ ! -d "grpc" ]; then
	git clone --branch v1.16.1 https://github.com/grpc/grpc.git
	cd grpc
	git submodule update --init
	make HAS_SYSTEM_PROTOBUF=true -j10
	make prefix=$BASEDIR/lib install
	#cd third_party/protobuf
	#make prefix=$BASEDIR/lib install
	# back in external
	cd $BASEDIR/external
fi

#G3Log installation
cd $BASEDIR/external
if [ ! -d "g3log" ]; then
	git clone --branch 1.3.2 https://github.com/KjellKod/g3log.git
	cd g3log
	cd 3rdParty/gtest
	unzip gtest-1.7.0.zip
	cd ../../
	mkdir build
	cd build
	cmake -DCPACK_PACKAGING_INSTALL_PREFIX=$BASEDIR/lib -DCMAKE_BUILD_TYPE=Release -DCHANGE_G3LOG_DEBUG_TO_DBUG=ON ..
	make -j10
	make install
fi


#libpqxx installation
cd $BASEDIR/external
if [ ! -d "libpqxx" ]; then
	git clone --branch 6.2.5 https://github.com/jtv/libpqxx.git
	cd libpqxx
	./configure --prefix=$BASEDIR/lib/ --disable-documentation
	make -j10
	make install
fi

#sgxsdk installation
cd $BASEDIR/external
if [ ! -d "linux-sgx" ]; then
	git clone https://github.com/intel/linux-sgx.git
	cd linux-sgx
	./download_prebuilt.sh
	make -j10 sdk_install_pkg
	cd ..
	(echo no; echo $BASEDIR/lib) | ./linux-sgx/linux/installer/bin/sgx_linux_x64_sdk*
fi

#ssl certs installation
cd $BASEDIR/build
cp ../ca.crt .
cp ../ca.key .
cp ../gen-certs.sh .
./gen-certs.sh



cd $BASEDIR
cd build
cmake ..
make -j10 vaultdb
make -j10
ctest 



