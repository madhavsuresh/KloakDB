# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/DB/vaultdb_operators

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/DB/vaultdb_operators

# Include any dependencies generated for this target.
include CMakeFiles/join_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/join_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/join_test.dir/flags.make

CMakeFiles/join_test.dir/join_test.cpp.o: CMakeFiles/join_test.dir/flags.make
CMakeFiles/join_test.dir/join_test.cpp.o: join_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/join_test.dir/join_test.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/join_test.dir/join_test.cpp.o -c /root/DB/vaultdb_operators/join_test.cpp

CMakeFiles/join_test.dir/join_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/join_test.dir/join_test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/join_test.cpp > CMakeFiles/join_test.dir/join_test.cpp.i

CMakeFiles/join_test.dir/join_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/join_test.dir/join_test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/join_test.cpp -o CMakeFiles/join_test.dir/join_test.cpp.s

CMakeFiles/join_test.dir/HashJoin.cpp.o: CMakeFiles/join_test.dir/flags.make
CMakeFiles/join_test.dir/HashJoin.cpp.o: HashJoin.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/join_test.dir/HashJoin.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/join_test.dir/HashJoin.cpp.o -c /root/DB/vaultdb_operators/HashJoin.cpp

CMakeFiles/join_test.dir/HashJoin.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/join_test.dir/HashJoin.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/HashJoin.cpp > CMakeFiles/join_test.dir/HashJoin.cpp.i

CMakeFiles/join_test.dir/HashJoin.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/join_test.dir/HashJoin.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/HashJoin.cpp -o CMakeFiles/join_test.dir/HashJoin.cpp.s

CMakeFiles/join_test.dir/postgres_client.cpp.o: CMakeFiles/join_test.dir/flags.make
CMakeFiles/join_test.dir/postgres_client.cpp.o: postgres_client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/join_test.dir/postgres_client.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/join_test.dir/postgres_client.cpp.o -c /root/DB/vaultdb_operators/postgres_client.cpp

CMakeFiles/join_test.dir/postgres_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/join_test.dir/postgres_client.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/postgres_client.cpp > CMakeFiles/join_test.dir/postgres_client.cpp.i

CMakeFiles/join_test.dir/postgres_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/join_test.dir/postgres_client.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/postgres_client.cpp -o CMakeFiles/join_test.dir/postgres_client.cpp.s

CMakeFiles/join_test.dir/Expressions.cpp.o: CMakeFiles/join_test.dir/flags.make
CMakeFiles/join_test.dir/Expressions.cpp.o: Expressions.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/DB/vaultdb_operators/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/join_test.dir/Expressions.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/join_test.dir/Expressions.cpp.o -c /root/DB/vaultdb_operators/Expressions.cpp

CMakeFiles/join_test.dir/Expressions.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/join_test.dir/Expressions.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/DB/vaultdb_operators/Expressions.cpp > CMakeFiles/join_test.dir/Expressions.cpp.i

CMakeFiles/join_test.dir/Expressions.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/join_test.dir/Expressions.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/DB/vaultdb_operators/Expressions.cpp -o CMakeFiles/join_test.dir/Expressions.cpp.s

# Object files for target join_test
join_test_OBJECTS = \
"CMakeFiles/join_test.dir/join_test.cpp.o" \
"CMakeFiles/join_test.dir/HashJoin.cpp.o" \
"CMakeFiles/join_test.dir/postgres_client.cpp.o" \
"CMakeFiles/join_test.dir/Expressions.cpp.o"

# External object files for target join_test
join_test_EXTERNAL_OBJECTS =

join_test: CMakeFiles/join_test.dir/join_test.cpp.o
join_test: CMakeFiles/join_test.dir/HashJoin.cpp.o
join_test: CMakeFiles/join_test.dir/postgres_client.cpp.o
join_test: CMakeFiles/join_test.dir/Expressions.cpp.o
join_test: CMakeFiles/join_test.dir/build.make
join_test: lib/libgtest_maind.a
join_test: lib/libgtestd.a
join_test: CMakeFiles/join_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/DB/vaultdb_operators/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable join_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/join_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/join_test.dir/build: join_test

.PHONY : CMakeFiles/join_test.dir/build

CMakeFiles/join_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/join_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/join_test.dir/clean

CMakeFiles/join_test.dir/depend:
	cd /root/DB/vaultdb_operators && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/DB/vaultdb_operators /root/DB/vaultdb_operators /root/DB/vaultdb_operators /root/DB/vaultdb_operators /root/DB/vaultdb_operators/CMakeFiles/join_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/join_test.dir/depend

