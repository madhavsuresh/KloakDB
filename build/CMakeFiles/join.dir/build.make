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
CMAKE_SOURCE_DIR = /home/mocco/vaultdb_operators

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mocco/vaultdb_operators/build

# Include any dependencies generated for this target.
include CMakeFiles/join.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/join.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/join.dir/flags.make

CMakeFiles/join.dir/SimpleJoin.cpp.o: CMakeFiles/join.dir/flags.make
CMakeFiles/join.dir/SimpleJoin.cpp.o: ../SimpleJoin.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mocco/vaultdb_operators/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/join.dir/SimpleJoin.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/join.dir/SimpleJoin.cpp.o -c /home/mocco/vaultdb_operators/SimpleJoin.cpp

CMakeFiles/join.dir/SimpleJoin.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/join.dir/SimpleJoin.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mocco/vaultdb_operators/SimpleJoin.cpp > CMakeFiles/join.dir/SimpleJoin.cpp.i

CMakeFiles/join.dir/SimpleJoin.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/join.dir/SimpleJoin.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mocco/vaultdb_operators/SimpleJoin.cpp -o CMakeFiles/join.dir/SimpleJoin.cpp.s

CMakeFiles/join.dir/test_simplejoin.cpp.o: CMakeFiles/join.dir/flags.make
CMakeFiles/join.dir/test_simplejoin.cpp.o: ../test_simplejoin.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mocco/vaultdb_operators/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/join.dir/test_simplejoin.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/join.dir/test_simplejoin.cpp.o -c /home/mocco/vaultdb_operators/test_simplejoin.cpp

CMakeFiles/join.dir/test_simplejoin.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/join.dir/test_simplejoin.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mocco/vaultdb_operators/test_simplejoin.cpp > CMakeFiles/join.dir/test_simplejoin.cpp.i

CMakeFiles/join.dir/test_simplejoin.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/join.dir/test_simplejoin.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mocco/vaultdb_operators/test_simplejoin.cpp -o CMakeFiles/join.dir/test_simplejoin.cpp.s

# Object files for target join
join_OBJECTS = \
"CMakeFiles/join.dir/SimpleJoin.cpp.o" \
"CMakeFiles/join.dir/test_simplejoin.cpp.o"

# External object files for target join
join_EXTERNAL_OBJECTS =

join: CMakeFiles/join.dir/SimpleJoin.cpp.o
join: CMakeFiles/join.dir/test_simplejoin.cpp.o
join: CMakeFiles/join.dir/build.make
join: CMakeFiles/join.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mocco/vaultdb_operators/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable join"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/join.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/join.dir/build: join

.PHONY : CMakeFiles/join.dir/build

CMakeFiles/join.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/join.dir/cmake_clean.cmake
.PHONY : CMakeFiles/join.dir/clean

CMakeFiles/join.dir/depend:
	cd /home/mocco/vaultdb_operators/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mocco/vaultdb_operators /home/mocco/vaultdb_operators /home/mocco/vaultdb_operators/build /home/mocco/vaultdb_operators/build /home/mocco/vaultdb_operators/build/CMakeFiles/join.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/join.dir/depend

