#pragma once

#include <GL/glew.h>
#include <iostream>
#include <string>

std::string readShaderFile(const std::string &filePath);
GLuint      createShaderProgram(const char *vertex, const char *fragment);
