# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /root/TestThreadPool/ThreadPool

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/TestThreadPool/ThreadPool/build

# Include any dependencies generated for this target.
include CMakeFiles/theadpool.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/theadpool.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/theadpool.dir/flags.make

CMakeFiles/theadpool.dir/example.cpp.o: CMakeFiles/theadpool.dir/flags.make
CMakeFiles/theadpool.dir/example.cpp.o: ../example.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/TestThreadPool/ThreadPool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/theadpool.dir/example.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/theadpool.dir/example.cpp.o -c /root/TestThreadPool/ThreadPool/example.cpp

CMakeFiles/theadpool.dir/example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/theadpool.dir/example.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/TestThreadPool/ThreadPool/example.cpp > CMakeFiles/theadpool.dir/example.cpp.i

CMakeFiles/theadpool.dir/example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/theadpool.dir/example.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/TestThreadPool/ThreadPool/example.cpp -o CMakeFiles/theadpool.dir/example.cpp.s

CMakeFiles/theadpool.dir/example.cpp.o.requires:

.PHONY : CMakeFiles/theadpool.dir/example.cpp.o.requires

CMakeFiles/theadpool.dir/example.cpp.o.provides: CMakeFiles/theadpool.dir/example.cpp.o.requires
	$(MAKE) -f CMakeFiles/theadpool.dir/build.make CMakeFiles/theadpool.dir/example.cpp.o.provides.build
.PHONY : CMakeFiles/theadpool.dir/example.cpp.o.provides

CMakeFiles/theadpool.dir/example.cpp.o.provides.build: CMakeFiles/theadpool.dir/example.cpp.o


# Object files for target theadpool
theadpool_OBJECTS = \
"CMakeFiles/theadpool.dir/example.cpp.o"

# External object files for target theadpool
theadpool_EXTERNAL_OBJECTS =

theadpool: CMakeFiles/theadpool.dir/example.cpp.o
theadpool: CMakeFiles/theadpool.dir/build.make
theadpool: CMakeFiles/theadpool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/TestThreadPool/ThreadPool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable theadpool"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/theadpool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/theadpool.dir/build: theadpool

.PHONY : CMakeFiles/theadpool.dir/build

CMakeFiles/theadpool.dir/requires: CMakeFiles/theadpool.dir/example.cpp.o.requires

.PHONY : CMakeFiles/theadpool.dir/requires

CMakeFiles/theadpool.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/theadpool.dir/cmake_clean.cmake
.PHONY : CMakeFiles/theadpool.dir/clean

CMakeFiles/theadpool.dir/depend:
	cd /root/TestThreadPool/ThreadPool/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/TestThreadPool/ThreadPool /root/TestThreadPool/ThreadPool /root/TestThreadPool/ThreadPool/build /root/TestThreadPool/ThreadPool/build /root/TestThreadPool/ThreadPool/build/CMakeFiles/theadpool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/theadpool.dir/depend

