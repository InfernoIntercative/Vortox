#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <GL/glew.h>

#include "struct.hpp"

extern char WTF_map[255];

bool L_loadLevel(const char *filename, std::vector<Sector> &sectors, std::vector<Wall> &walls,
                 std::unordered_map<std::string, GLuint> &textureLevel, Spawn &spawn);
