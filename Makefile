CXX ?= ccache g++
CXXFLAGS := -std=c++20 -Wall -Wpedantic -Wextra -pipe -MMD -fno-exceptions -fno-rtti -fomit-frame-pointer -fstrict-aliasing
BUILD ?= Release
JOBS ?= $(shell nproc)

# optimize ccache
export CCACHE_COMPRESS=1
export CCACHE_COMPRESSLEVEL=6
export CCACHE_MAXSIZE=5G
export CCACHE_SLOPPINESS=pch_defines,time_macros

# enable/disable precompiled headers (default: disabled)
USE_PCH ?= 0

ifeq ($(BUILD),Release)
    CXXFLAGS += -O3 -march=native -flto
else ifeq ($(BUILD),Debug)
    CXXFLAGS += -g -DDEBUG -Og -fno-omit-frame-pointer -fno-inline -fstack-protector-all
    # uncomment the following lines for additional debugging tools
    # CXXFLAGS += -fsanitize=address -fsanitize=undefined
    # LDFLAGS += -fsanitize=address -fsanitize=undefined
else ifeq ($(BUILD),Profile)
    CXXFLAGS += -O3 -g -pg -DNDEBUG
endif

# enable faster linking when not using LTO
ifneq ($(BUILD),Release)
    LDFLAGS += -Wl,--as-needed
endif

SDL2_CFLAGS   := $(shell sdl2-config --cflags)
SDL2_LIBS     := $(shell sdl2-config --libs)
GLEW_CFLAGS   := $(shell pkg-config --cflags glew)
GLEW_LIBS     := $(shell pkg-config --libs glew)
SDL2_TTF_CFLAGS  := $(shell pkg-config --cflags SDL2_ttf)
SDL2_TTF_LIBS    := $(shell pkg-config --libs SDL2_ttf)
SDL2_IMAGE_LIBS  := $(shell pkg-config --libs SDL2_image)
SDL2_MIXER_LIBS  := $(shell pkg-config --libs SDL2_mixer)
OPENGL_LIBS   := -lGL

INCLUDES := $(SDL2_CFLAGS) $(GLEW_CFLAGS) $(SDL2_TTF_CFLAGS)
LIBS     := $(SDL2_LIBS) $(GLEW_LIBS) $(SDL2_TTF_LIBS) $(SDL2_IMAGE_LIBS) $(SDL2_MIXER_LIBS) $(OPENGL_LIBS)

SRC_DIR := src
OBJ_DIR := build/obj/$(BUILD)
BIN_DIR := build/bin
EXEC    := Vortox

# precompiled header setup
PCH_DIR := $(OBJ_DIR)/pch
PCH_SRC := $(SRC_DIR)/pch.h
PCH_GCH := $(PCH_DIR)/pch.h.gch

# unity build support
UNITY_BUILD ?= 0
UNITY_SIZE ?= 10
UNITY_DIR := $(OBJ_DIR)/unity

SOURCES := $(shell find $(SRC_DIR) -type f -name "*.cpp" | grep -v "/lib/")
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS    := $(OBJS:.o=.d)

# unity build objects
ifeq ($(UNITY_BUILD),1)
    UNITY_SOURCES := $(shell echo $(SOURCES) | tr ' ' '\n' | sort)
    UNITY_COUNT := $(shell echo $(UNITY_SOURCES) | wc -w)
    UNITY_CHUNKS := $(shell expr $(UNITY_COUNT) / $(UNITY_SIZE) + 1)
    UNITY_OBJS := $(foreach i,$(shell seq 1 $(UNITY_CHUNKS)),$(UNITY_DIR)/chunk_$(i).o)
    FINAL_OBJS := $(UNITY_OBJS)
else
    FINAL_OBJS := $(OBJS)
endif

# check if pch.h exists
PCH_EXISTS := $(shell test -f $(PCH_SRC) && echo 1 || echo 0)
ifeq ($(USE_PCH),1)
    ifeq ($(PCH_EXISTS),1)
        USE_PCH_ACTUAL := 1
    else
        USE_PCH_ACTUAL := 0
        $(warning PCH file $(PCH_SRC) not found. Disabling precompiled headers.)
    endif
else
    USE_PCH_ACTUAL := 0
endif

.PHONY: all
all: $(BIN_DIR)/$(EXEC)

# precompiled header rule
ifeq ($(USE_PCH_ACTUAL),1)
$(PCH_GCH): $(PCH_SRC)
	@mkdir -p $(PCH_DIR)
	@echo "Precompiling header $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -x c++-header $< -o $@
endif

# unity build rules
ifeq ($(UNITY_BUILD),1)
$(UNITY_DIR)/chunk_%.cpp:
	@mkdir -p $(UNITY_DIR)
	@echo "Generating unity chunk $@..."
	@start=$$(( ($* - 1) * $(UNITY_SIZE) + 1 )); \
	end=$$(( $* * $(UNITY_SIZE) )); \
	if [ $$end -gt $(UNITY_COUNT) ]; then end=$(UNITY_COUNT); fi; \
	echo "// Unity build chunk $*" > $@; \
	for i in $$(seq $$start $$end); do \
		file=$$(echo $(UNITY_SOURCES) | tr ' ' '\n' | sed -n "$${i}p"); \
		echo "#include \"$${file#src/}\"" >> $@; \
	done

ifeq ($(USE_PCH_ACTUAL),1)
$(UNITY_DIR)/chunk_%.o: $(UNITY_DIR)/chunk_%.cpp $(PCH_GCH)
	@echo "Compiling unity chunk $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -include $(PCH_SRC) -c $< -o $@
else
$(UNITY_DIR)/chunk_%.o: $(UNITY_DIR)/chunk_%.cpp
	@echo "Compiling unity chunk $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
endif

unity-chunks: $(foreach i,$(shell seq 1 $(UNITY_CHUNKS)),$(UNITY_DIR)/chunk_$(i).cpp)
endif

$(BIN_DIR)/$(EXEC): $(FINAL_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking $@..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
	@echo "Build complete: $@"

ifeq ($(USE_PCH_ACTUAL),1)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(PCH_GCH)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -include $(PCH_SRC) -c $< -o $@
else
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
endif

-include $(DEPS)

.PHONY: clean clean-obj clean-all ccache-clear
clean:
	@echo "Cleaning build files..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)/$(EXEC)
	@echo "Yeah! Clean complete!"

clean-obj:
	@echo "Cleaning object files..."
	@rm -rf $(OBJ_DIR)
	@echo "Yeah! Object files cleaned!"

clean-all:
	@echo "Cleaning all build directories..."
	@rm -rf build
	@echo "Yeah! All build directories cleaned!"

ccache-clear:
	@echo "Clearing ccache..."
	@ccache -C
	@echo "ccache cleared!"

.PHONY: rebuild
rebuild: clean all

.PHONY: run
run: all
	./$(BIN_DIR)/$(EXEC)

.PHONY: parallel
parallel:
	@$(MAKE) -j$(JOBS) all

.PHONY: ccache-stats
ccache-stats:
	@ccache -s

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all          - Build the executable (default)"
	@echo "  clean        - Remove object files and executable"
	@echo "  clean-obj    - Remove only object files"
	@echo "  clean-all    - Remove all build directories"
	@echo "  ccache-clear - Clear ccache cache"
	@echo "  ccache-stats - Show ccache statistics"
	@echo "  rebuild      - Clean and rebuild all"
	@echo "  run          - Build and run the executable"
	@echo "  parallel     - Build using multiple jobs (default: $(JOBS))"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  make BUILD=Release    - Optimized build (default)"
	@echo "  make BUILD=Debug      - Debug build"
	@echo "  make BUILD=Profile    - Profiling build"
	@echo "  make JOBS=N           - Use N parallel jobs for 'parallel' target"
	@echo "  make UNITY_BUILD=1    - Enable unity build (combine source files)"
	@echo "  make UNITY_SIZE=N     - Number of files per unity chunk (default: 10)"
	@echo "  make USE_PCH=1        - Enable precompiled headers (requires src/pch.h)"

.DEFAULT_GOAL := parallel
