# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/russian95/Xylon

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/russian95/Xylon

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/russian95/Xylon/CMakeFiles /home/russian95/Xylon//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/russian95/Xylon/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named XylonEngine

# Build rule for target.
XylonEngine: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 XylonEngine
.PHONY : XylonEngine

# fast build rule for target.
XylonEngine/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/build
.PHONY : XylonEngine/fast

#=============================================================================
# Target rules for targets named clean_project

# Build rule for target.
clean_project: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean_project
.PHONY : clean_project

# fast build rule for target.
clean_project/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/clean_project.dir/build.make CMakeFiles/clean_project.dir/build
.PHONY : clean_project/fast

src/console/console.o: src/console/console.cpp.o
.PHONY : src/console/console.o

# target to build an object file
src/console/console.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/console/console.cpp.o
.PHONY : src/console/console.cpp.o

src/console/console.i: src/console/console.cpp.i
.PHONY : src/console/console.i

# target to preprocess a source file
src/console/console.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/console/console.cpp.i
.PHONY : src/console/console.cpp.i

src/console/console.s: src/console/console.cpp.s
.PHONY : src/console/console.s

# target to generate assembly for a file
src/console/console.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/console/console.cpp.s
.PHONY : src/console/console.cpp.s

src/core/globals.o: src/core/globals.cpp.o
.PHONY : src/core/globals.o

# target to build an object file
src/core/globals.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/core/globals.cpp.o
.PHONY : src/core/globals.cpp.o

src/core/globals.i: src/core/globals.cpp.i
.PHONY : src/core/globals.i

# target to preprocess a source file
src/core/globals.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/core/globals.cpp.i
.PHONY : src/core/globals.cpp.i

src/core/globals.s: src/core/globals.cpp.s
.PHONY : src/core/globals.s

# target to generate assembly for a file
src/core/globals.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/core/globals.cpp.s
.PHONY : src/core/globals.cpp.s

src/core/main.o: src/core/main.cpp.o
.PHONY : src/core/main.o

# target to build an object file
src/core/main.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/core/main.cpp.o
.PHONY : src/core/main.cpp.o

src/core/main.i: src/core/main.cpp.i
.PHONY : src/core/main.i

# target to preprocess a source file
src/core/main.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/core/main.cpp.i
.PHONY : src/core/main.cpp.i

src/core/main.s: src/core/main.cpp.s
.PHONY : src/core/main.s

# target to generate assembly for a file
src/core/main.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/core/main.cpp.s
.PHONY : src/core/main.cpp.s

src/errors/error.o: src/errors/error.cpp.o
.PHONY : src/errors/error.o

# target to build an object file
src/errors/error.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/errors/error.cpp.o
.PHONY : src/errors/error.cpp.o

src/errors/error.i: src/errors/error.cpp.i
.PHONY : src/errors/error.i

# target to preprocess a source file
src/errors/error.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/errors/error.cpp.i
.PHONY : src/errors/error.cpp.i

src/errors/error.s: src/errors/error.cpp.s
.PHONY : src/errors/error.s

# target to generate assembly for a file
src/errors/error.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/errors/error.cpp.s
.PHONY : src/errors/error.cpp.s

src/fonts/fonts.o: src/fonts/fonts.cpp.o
.PHONY : src/fonts/fonts.o

# target to build an object file
src/fonts/fonts.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/fonts/fonts.cpp.o
.PHONY : src/fonts/fonts.cpp.o

src/fonts/fonts.i: src/fonts/fonts.cpp.i
.PHONY : src/fonts/fonts.i

# target to preprocess a source file
src/fonts/fonts.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/fonts/fonts.cpp.i
.PHONY : src/fonts/fonts.cpp.i

src/fonts/fonts.s: src/fonts/fonts.cpp.s
.PHONY : src/fonts/fonts.s

# target to generate assembly for a file
src/fonts/fonts.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/fonts/fonts.cpp.s
.PHONY : src/fonts/fonts.cpp.s

src/graphics/texture.o: src/graphics/texture.cpp.o
.PHONY : src/graphics/texture.o

# target to build an object file
src/graphics/texture.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/graphics/texture.cpp.o
.PHONY : src/graphics/texture.cpp.o

src/graphics/texture.i: src/graphics/texture.cpp.i
.PHONY : src/graphics/texture.i

# target to preprocess a source file
src/graphics/texture.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/graphics/texture.cpp.i
.PHONY : src/graphics/texture.cpp.i

src/graphics/texture.s: src/graphics/texture.cpp.s
.PHONY : src/graphics/texture.s

# target to generate assembly for a file
src/graphics/texture.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/graphics/texture.cpp.s
.PHONY : src/graphics/texture.cpp.s

src/levels/load.o: src/levels/load.cpp.o
.PHONY : src/levels/load.o

# target to build an object file
src/levels/load.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/levels/load.cpp.o
.PHONY : src/levels/load.cpp.o

src/levels/load.i: src/levels/load.cpp.i
.PHONY : src/levels/load.i

# target to preprocess a source file
src/levels/load.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/levels/load.cpp.i
.PHONY : src/levels/load.cpp.i

src/levels/load.s: src/levels/load.cpp.s
.PHONY : src/levels/load.s

# target to generate assembly for a file
src/levels/load.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/levels/load.cpp.s
.PHONY : src/levels/load.cpp.s

src/player/collision.o: src/player/collision.cpp.o
.PHONY : src/player/collision.o

# target to build an object file
src/player/collision.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/collision.cpp.o
.PHONY : src/player/collision.cpp.o

src/player/collision.i: src/player/collision.cpp.i
.PHONY : src/player/collision.i

# target to preprocess a source file
src/player/collision.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/collision.cpp.i
.PHONY : src/player/collision.cpp.i

src/player/collision.s: src/player/collision.cpp.s
.PHONY : src/player/collision.s

# target to generate assembly for a file
src/player/collision.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/collision.cpp.s
.PHONY : src/player/collision.cpp.s

src/player/input.o: src/player/input.cpp.o
.PHONY : src/player/input.o

# target to build an object file
src/player/input.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/input.cpp.o
.PHONY : src/player/input.cpp.o

src/player/input.i: src/player/input.cpp.i
.PHONY : src/player/input.i

# target to preprocess a source file
src/player/input.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/input.cpp.i
.PHONY : src/player/input.cpp.i

src/player/input.s: src/player/input.cpp.s
.PHONY : src/player/input.s

# target to generate assembly for a file
src/player/input.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/input.cpp.s
.PHONY : src/player/input.cpp.s

src/player/keyboard.o: src/player/keyboard.cpp.o
.PHONY : src/player/keyboard.o

# target to build an object file
src/player/keyboard.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/keyboard.cpp.o
.PHONY : src/player/keyboard.cpp.o

src/player/keyboard.i: src/player/keyboard.cpp.i
.PHONY : src/player/keyboard.i

# target to preprocess a source file
src/player/keyboard.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/keyboard.cpp.i
.PHONY : src/player/keyboard.cpp.i

src/player/keyboard.s: src/player/keyboard.cpp.s
.PHONY : src/player/keyboard.s

# target to generate assembly for a file
src/player/keyboard.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/keyboard.cpp.s
.PHONY : src/player/keyboard.cpp.s

src/player/mouse.o: src/player/mouse.cpp.o
.PHONY : src/player/mouse.o

# target to build an object file
src/player/mouse.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/mouse.cpp.o
.PHONY : src/player/mouse.cpp.o

src/player/mouse.i: src/player/mouse.cpp.i
.PHONY : src/player/mouse.i

# target to preprocess a source file
src/player/mouse.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/mouse.cpp.i
.PHONY : src/player/mouse.cpp.i

src/player/mouse.s: src/player/mouse.cpp.s
.PHONY : src/player/mouse.s

# target to generate assembly for a file
src/player/mouse.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/player/mouse.cpp.s
.PHONY : src/player/mouse.cpp.s

src/shaders/shaders.o: src/shaders/shaders.cpp.o
.PHONY : src/shaders/shaders.o

# target to build an object file
src/shaders/shaders.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/shaders/shaders.cpp.o
.PHONY : src/shaders/shaders.cpp.o

src/shaders/shaders.i: src/shaders/shaders.cpp.i
.PHONY : src/shaders/shaders.i

# target to preprocess a source file
src/shaders/shaders.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/shaders/shaders.cpp.i
.PHONY : src/shaders/shaders.cpp.i

src/shaders/shaders.s: src/shaders/shaders.cpp.s
.PHONY : src/shaders/shaders.s

# target to generate assembly for a file
src/shaders/shaders.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/XylonEngine.dir/build.make CMakeFiles/XylonEngine.dir/src/shaders/shaders.cpp.s
.PHONY : src/shaders/shaders.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... clean_project"
	@echo "... XylonEngine"
	@echo "... src/console/console.o"
	@echo "... src/console/console.i"
	@echo "... src/console/console.s"
	@echo "... src/core/globals.o"
	@echo "... src/core/globals.i"
	@echo "... src/core/globals.s"
	@echo "... src/core/main.o"
	@echo "... src/core/main.i"
	@echo "... src/core/main.s"
	@echo "... src/errors/error.o"
	@echo "... src/errors/error.i"
	@echo "... src/errors/error.s"
	@echo "... src/fonts/fonts.o"
	@echo "... src/fonts/fonts.i"
	@echo "... src/fonts/fonts.s"
	@echo "... src/graphics/texture.o"
	@echo "... src/graphics/texture.i"
	@echo "... src/graphics/texture.s"
	@echo "... src/levels/load.o"
	@echo "... src/levels/load.i"
	@echo "... src/levels/load.s"
	@echo "... src/player/collision.o"
	@echo "... src/player/collision.i"
	@echo "... src/player/collision.s"
	@echo "... src/player/input.o"
	@echo "... src/player/input.i"
	@echo "... src/player/input.s"
	@echo "... src/player/keyboard.o"
	@echo "... src/player/keyboard.i"
	@echo "... src/player/keyboard.s"
	@echo "... src/player/mouse.o"
	@echo "... src/player/mouse.i"
	@echo "... src/player/mouse.s"
	@echo "... src/shaders/shaders.o"
	@echo "... src/shaders/shaders.i"
	@echo "... src/shaders/shaders.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

