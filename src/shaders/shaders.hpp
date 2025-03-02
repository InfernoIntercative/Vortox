#pragma once

#include <iostream>
#include <GL/glew.h>
#include <string>

std::string readShaderFile(const std::string &filePath);
GLuint createShaderProgram(const char *vertex, const char *fragment);
