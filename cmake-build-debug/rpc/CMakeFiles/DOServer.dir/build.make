# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /root/Downloads/CLion-2018.2.4/clion-2018.2.4/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /root/Downloads/CLion-2018.2.4/clion-2018.2.4/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/DB/vaultdb_operators

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/DB/vaultdb_operators/cmake-build-debug

# Include any dependencies generated for this target.
include rpc/CMakeFiles/DOServer.dir/depend.make

# Include the progress variables for this target.
include rpc/CMakeFiles/DOServer.dir/progress.make

# Include the compile flags for this target's objects.
include rpc/CMakeFiles/DOServer.dir/flags.make

rpc/vaultdb.grpc.pb.cc:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating vaultdb.grpc.pb.cc"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && protoc -I /root/DB/vaultdb_operators/rpc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` /root/DB/vaultdb_operators/rpc/vaultdb.proto

rpc/vaultdb.pb.cc:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating vaultdb.pb.cc"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && protoc -I /root/DB/vaultdb_operators/rpc --cpp_out=. /root/DB/vaultdb_operators/rpc/vaultdb.proto

rpc/CMakeFiles/DOServer.dir/DOServer.cpp.o: rpc/CMakeFiles/DOServer.dir/flags.make
rpc/CMakeFiles/DOServer.dir/DOServer.cpp.o: ../rpc/DOServer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object rpc/CMakeFiles/DOServer.dir/DOServer.cpp.o"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DOServer.dir/DOServer.cpp.o -c /root/DB/vaultdb_operators/rpc/DOServer.cpp

rpc/CMakeFiles/DOServer.dir/DOServer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DOServer.dir/DOServer.cpp.i"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/rpc/DOServer.cpp > CMakeFiles/DOServer.dir/DOServer.cpp.i

rpc/CMakeFiles/DOServer.dir/DOServer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DOServer.dir/DOServer.cpp.s"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/rpc/DOServer.cpp -o CMakeFiles/DOServer.dir/DOServer.cpp.s

rpc/CMakeFiles/DOServer.dir/__/postgres_client.cpp.o: rpc/CMakeFiles/DOServer.dir/flags.make
rpc/CMakeFiles/DOServer.dir/__/postgres_client.cpp.o: ../postgres_client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object rpc/CMakeFiles/DOServer.dir/__/postgres_client.cpp.o"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DOServer.dir/__/postgres_client.cpp.o -c /root/DB/vaultdb_operators/postgres_client.cpp

rpc/CMakeFiles/DOServer.dir/__/postgres_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DOServer.dir/__/postgres_client.cpp.i"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/postgres_client.cpp > CMakeFiles/DOServer.dir/__/postgres_client.cpp.i

rpc/CMakeFiles/DOServer.dir/__/postgres_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DOServer.dir/__/postgres_client.cpp.s"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/postgres_client.cpp -o CMakeFiles/DOServer.dir/__/postgres_client.cpp.s

rpc/CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.o: rpc/CMakeFiles/DOServer.dir/flags.make
rpc/CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.o: rpc/vaultdb.grpc.pb.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object rpc/CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.o"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.o -c /root/DB/vaultdb_operators/cmake-build-debug/rpc/vaultdb.grpc.pb.cc

rpc/CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.i"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/cmake-build-debug/rpc/vaultdb.grpc.pb.cc > CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.i

rpc/CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.s"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/cmake-build-debug/rpc/vaultdb.grpc.pb.cc -o CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.s

rpc/CMakeFiles/DOServer.dir/vaultdb.pb.cc.o: rpc/CMakeFiles/DOServer.dir/flags.make
rpc/CMakeFiles/DOServer.dir/vaultdb.pb.cc.o: rpc/vaultdb.pb.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object rpc/CMakeFiles/DOServer.dir/vaultdb.pb.cc.o"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DOServer.dir/vaultdb.pb.cc.o -c /root/DB/vaultdb_operators/cmake-build-debug/rpc/vaultdb.pb.cc

rpc/CMakeFiles/DOServer.dir/vaultdb.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DOServer.dir/vaultdb.pb.cc.i"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/cmake-build-debug/rpc/vaultdb.pb.cc > CMakeFiles/DOServer.dir/vaultdb.pb.cc.i

rpc/CMakeFiles/DOServer.dir/vaultdb.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DOServer.dir/vaultdb.pb.cc.s"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/cmake-build-debug/rpc/vaultdb.pb.cc -o CMakeFiles/DOServer.dir/vaultdb.pb.cc.s

# Object files for target DOServer
DOServer_OBJECTS = \
"CMakeFiles/DOServer.dir/DOServer.cpp.o" \
"CMakeFiles/DOServer.dir/__/postgres_client.cpp.o" \
"CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.o" \
"CMakeFiles/DOServer.dir/vaultdb.pb.cc.o"

# External object files for target DOServer
DOServer_EXTERNAL_OBJECTS =

rpc/DOServer: rpc/CMakeFiles/DOServer.dir/DOServer.cpp.o
rpc/DOServer: rpc/CMakeFiles/DOServer.dir/__/postgres_client.cpp.o
rpc/DOServer: rpc/CMakeFiles/DOServer.dir/vaultdb.grpc.pb.cc.o
rpc/DOServer: rpc/CMakeFiles/DOServer.dir/vaultdb.pb.cc.o
rpc/DOServer: rpc/CMakeFiles/DOServer.dir/build.make
rpc/DOServer: rpc/libHBGeneralize.a
rpc/DOServer: rpc/CMakeFiles/DOServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/DB/vaultdb_operators/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable DOServer"
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DOServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
rpc/CMakeFiles/DOServer.dir/build: rpc/DOServer

.PHONY : rpc/CMakeFiles/DOServer.dir/build

rpc/CMakeFiles/DOServer.dir/clean:
	cd /root/DB/vaultdb_operators/cmake-build-debug/rpc && $(CMAKE_COMMAND) -P CMakeFiles/DOServer.dir/cmake_clean.cmake
.PHONY : rpc/CMakeFiles/DOServer.dir/clean

rpc/CMakeFiles/DOServer.dir/depend: rpc/vaultdb.grpc.pb.cc
rpc/CMakeFiles/DOServer.dir/depend: rpc/vaultdb.pb.cc
	cd /root/DB/vaultdb_operators/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/DB/vaultdb_operators /root/DB/vaultdb_operators/rpc /root/DB/vaultdb_operators/cmake-build-debug /root/DB/vaultdb_operators/cmake-build-debug/rpc /root/DB/vaultdb_operators/cmake-build-debug/rpc/CMakeFiles/DOServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : rpc/CMakeFiles/DOServer.dir/depend

