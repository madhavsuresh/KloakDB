git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git checkout tags/v3.6.1
git submodule update --init --recursive
./autogen.sh
./configure
make -j10
make check -j10

