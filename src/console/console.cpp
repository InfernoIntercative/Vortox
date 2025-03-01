#include <stdio.h>
#include <iostream>

// opengl
#include <GL/glew.h>

// headers
#include "console.hpp"

// important, super important!
bool G_Paused = false;
bool consoleActive = false;
float consoleAnim = 0.0f;

std::string commandInput = "";
const float consoleAnimSpeed = 5.0f;
const int consoleHeight = 300;

const char *console_vertex_shader_source = R"(
    #version 330 core
    layout(location = 0) in vec2 position;
    uniform mat4 model;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * model * vec4(position, 0.0, 1.0);
    }
    )";

const char *console_fragment_shader_source = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;
    void main() {
        FragColor = color;
    }
    )";


