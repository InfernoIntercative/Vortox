#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

extern const char *consoleVertexShaderSource;

extern const char *consoleFragmentShaderSource;

GLuint createConsoleShaderProgram();
GLuint createTextShaderProgram();

extern bool G_Paused;
extern bool consoleActive;
extern float consoleAnim;
extern const float consoleAnimSpeed;
extern const int consoleHeight;
