CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc`\
           -Wl,--no-as-needed -Wl,--as-needed\
           -ldl -lpqxx
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

rpc_protos:
	protoc -I . --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` vaultdb.proto
	protoc -I . --cpp_out=. vaultdb.proto

vaultdb_do: vaultdb.pb.o vaultdb.grpc.pb.o DOServer.o ../postgres_client.o
	$(CXX)  $^ $(LDFLAGS) -o $@

vaultdb_client: vaultdb.pb.o vaultdb.grpc.pb.o DOClient.o 
	$(CXX)  $^ $(LDFLAGS) -o $@
