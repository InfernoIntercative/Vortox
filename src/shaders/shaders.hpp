#pragma once

std::string readShaderFile(const std::string &filePath);
GLuint createShaderProgram(const char *vertex, const char *fragment);
