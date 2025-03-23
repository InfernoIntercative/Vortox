#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>

// opengl
#include <GL/glew.h>

// headers
#include "../logsystem/log.hpp"

std::string readShaderFile(const std::string &filePath) {
    if (filePath.empty()) {
        panic("Shader file path is empty!");
    }

    std::ifstream     shaderFile(filePath);
    std::stringstream shaderStream;
    if (!shaderFile.is_open()) {
        panic("Could not open shader file", filePath.c_str());
    }
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

// why? why i did this?
GLuint createShaderProgram(const char *vertex, const char *fragment) {
    if (!vertex || vertex[0] == '\0' || !fragment || fragment[0] == '\0') {
        panic("Shader source not loaded.");
    }
    GLuint      vertexShader     = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderCode = vertex;
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        panic("Error compiling vertex shader", infoLog);
    }
    GLuint      fragmentShader     = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderCode = fragment;
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        panic("Error compiling fragment shader", infoLog);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        panic("Error linking shader program", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
