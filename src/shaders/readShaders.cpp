#include <cstdio>

// headers
#include "readShaders.hpp"
#include "shaders.hpp"

// this is hardcoded for now, but i will implement a better way to load shaders later

// main shaders
std::string main_vertex_shader_source =
    readShaderFile("shaders/main/vertex.vert");

std::string main_fragment_shader_source =
    readShaderFile("shaders/main/fragment.frag");

// text shaders
std::string text_vertex_shader_source =
    readShaderFile("shaders/text/vertex.vert");

std::string text_fragment_shader_source =
    readShaderFile("shaders/text/fragment.frag");

// console shaders
std::string console_vertex_shader_source =
    readShaderFile("shaders/console/vertex.vert");

std::string console_fragment_shader_source =
    readShaderFile("shaders/console/fragment.frag");