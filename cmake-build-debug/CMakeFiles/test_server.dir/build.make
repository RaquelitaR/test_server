# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/raquelitarosaguilar/CLionProjects/test_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/test_server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_server.dir/flags.make

CMakeFiles/test_server.dir/src/httpd.c.o: CMakeFiles/test_server.dir/flags.make
CMakeFiles/test_server.dir/src/httpd.c.o: ../src/httpd.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/test_server.dir/src/httpd.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test_server.dir/src/httpd.c.o   -c /Users/raquelitarosaguilar/CLionProjects/test_server/src/httpd.c

CMakeFiles/test_server.dir/src/httpd.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_server.dir/src/httpd.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/raquelitarosaguilar/CLionProjects/test_server/src/httpd.c > CMakeFiles/test_server.dir/src/httpd.c.i

CMakeFiles/test_server.dir/src/httpd.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_server.dir/src/httpd.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/raquelitarosaguilar/CLionProjects/test_server/src/httpd.c -o CMakeFiles/test_server.dir/src/httpd.c.s

CMakeFiles/test_server.dir/src/httpd.c.o.requires:

.PHONY : CMakeFiles/test_server.dir/src/httpd.c.o.requires

CMakeFiles/test_server.dir/src/httpd.c.o.provides: CMakeFiles/test_server.dir/src/httpd.c.o.requires
	$(MAKE) -f CMakeFiles/test_server.dir/build.make CMakeFiles/test_server.dir/src/httpd.c.o.provides.build
.PHONY : CMakeFiles/test_server.dir/src/httpd.c.o.provides

CMakeFiles/test_server.dir/src/httpd.c.o.provides.build: CMakeFiles/test_server.dir/src/httpd.c.o


# Object files for target test_server
test_server_OBJECTS = \
"CMakeFiles/test_server.dir/src/httpd.c.o"

# External object files for target test_server
test_server_EXTERNAL_OBJECTS =

test_server: CMakeFiles/test_server.dir/src/httpd.c.o
test_server: CMakeFiles/test_server.dir/build.make
test_server: CMakeFiles/test_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable test_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_server.dir/build: test_server

.PHONY : CMakeFiles/test_server.dir/build

CMakeFiles/test_server.dir/requires: CMakeFiles/test_server.dir/src/httpd.c.o.requires

.PHONY : CMakeFiles/test_server.dir/requires

CMakeFiles/test_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_server.dir/clean

CMakeFiles/test_server.dir/depend:
	cd /Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/raquelitarosaguilar/CLionProjects/test_server /Users/raquelitarosaguilar/CLionProjects/test_server /Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug /Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug /Users/raquelitarosaguilar/CLionProjects/test_server/cmake-build-debug/CMakeFiles/test_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_server.dir/depend

