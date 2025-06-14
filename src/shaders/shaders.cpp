#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// OpenGL
#include <GL/glew.h>

// headers
#include "../logsystem/log.hpp"

std::string readShaderFile(const std::string &filePath) {
    if (filePath.empty()) {
        panic("Shader file path is empty");
    }

    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        panic("Could not open shader file", filePath.c_str());
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

// add overload for const char*
std::string readShaderFile(const char* filePath) {
    if (filePath == nullptr) {
        panic("Shader file path is null");
    }
    return readShaderFile(std::string(filePath));
}

GLuint compileShader(GLenum shaderType, const char *shaderSource) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        const char *shaderTypeStr = nullptr;
        switch (shaderType) {
            case GL_VERTEX_SHADER: shaderTypeStr = "vertex"; break;
            case GL_FRAGMENT_SHADER: shaderTypeStr = "fragment"; break;
            case GL_GEOMETRY_SHADER: shaderTypeStr = "geometry"; break;
            default: shaderTypeStr = "unknown"; break;
        }
        std::string errMsg = "Error compiling " + std::string(shaderTypeStr) + " shader: " + infoLog;
        panic(errMsg.c_str());
    }

    return shader;
}

GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::string errMsg = std::string("Error linking shader program: ") + infoLog;
        panic(errMsg.c_str());
    }

    // delete shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint linkMultipleShaders(const std::vector<GLuint> &shaders) {
    GLuint shaderProgram = glCreateProgram();
    for (const auto &shader : shaders) {
        glAttachShader(shaderProgram, shader);
    }
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::string errMsg = std::string("Error linking shader program: ") + infoLog;
        panic(errMsg.c_str());
    }

    // delete shaders after linking
    for (const auto &shader : shaders) {
        glDeleteShader(shader);
    }

    return shaderProgram;
}

GLuint createShaderProgram(const std::vector<std::pair<GLenum, const char *>> &shaderSources) {
    if (shaderSources.empty()) {
        panic("No shader sources provided.");
    }

    std::vector<GLuint> compiledShaders;
    for (const auto &[shaderType, shaderSource] : shaderSources) {
        if (!shaderSource || shaderSource[0] == '\0') {
            panic("Shader source not loaded for shader type: ", shaderType);
        }
        compiledShaders.push_back(compileShader(shaderType, shaderSource));
    }

    return linkMultipleShaders(compiledShaders);
}
