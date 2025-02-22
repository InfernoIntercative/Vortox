#!/bin/bash

clear

g++ -o level_editor/build/XYM_LEditor level_editor/editor.cpp \
    lib/imgui/imgui.cpp lib/imgui/imgui_draw.cpp lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp \
    lib/imgui/imgui_impl_sdl2.cpp lib/imgui/imgui_impl_opengl3.cpp \
    $(pkg-config --cflags --libs sdl2 SDL2_ttf) \
    -lGL -lGLEW -lGLU -ldl

./level_editor/build/XYM_LEditor
