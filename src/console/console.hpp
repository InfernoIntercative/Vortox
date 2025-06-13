#pragma once

// OpenGL
#include <GL/gl.h>
#include <GL/glew.h>

// headers
#include "../levels/struct.hpp"

// the boys
#include <string>
#include <vector>

#include <unordered_map>

extern bool  G_Paused;
extern bool  consoleActive;
extern float consoleAnim;
extern float consoleAnimSpeed;
extern int   consoleHeight;

void processCommand(const std::string &cmd, bool &G_running,
                    std::vector<Sector> &sectors, std::vector<Wall> &walls,
                    std::vector<float> &levelVertices, GLuint VBO,
                    std::unordered_map<std::string, GLuint> &textureLevel,
                    float &mouseSensitivity, int &currentVolume, SDL_Window *window,
                    std::vector<std::string> &commandHistory, int &historyIndex);