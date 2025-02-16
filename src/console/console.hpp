#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

const char *consoleVertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 position;
    uniform mat4 model;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * model * vec4(position, 0.0, 1.0);
    }
    )";

const char *consoleFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;
    void main() {
        FragColor = color;
    }
    )";

GLuint createConsoleShaderProgram();
GLuint createTextShaderProgram();

extern bool gPaused;
extern bool consoleActive;
extern float consoleAnim;
extern const float consoleAnimSpeed;
extern const int consoleHeight;
