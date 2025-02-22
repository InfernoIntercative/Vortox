#pragma once

const char* readShaderFile(const std::string& filePath);
GLuint createShaderProgram(const char* vertex, const char* fragment);
