CXX ?= ccache g++
CXXFLAGS := -std=c++17 -pipe -MMD
BUILD ?= Release
ifeq ($(BUILD),Release)
    CXXFLAGS += -O3 -march=native -flto
else ifeq ($(BUILD),Debug)
    CXXFLAGS += -g -DDEBUG
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
OBJ_DIR := build/obj
BIN_DIR := build/bin
EXEC    := XylonEngine

SOURCES := $(shell find $(SRC_DIR) -type f -name "*.cpp" | grep -v "/lib/")
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS    := $(OBJS:.o=.d)

all: $(BIN_DIR)/$(EXEC)

$(BIN_DIR)/$(EXEC): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)

.PHONY: clean run
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Yeah, clean!"

run: all
	./$(BIN_DIR)/$(EXEC)
