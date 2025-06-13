#pragma once

#include <GL/glew.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "struct.hpp"

extern char L_active_map_path[];

bool L_LoadLevel(const char *filename, std::vector<Sector> &sectors,
                 std::vector<Wall>                       &walls,
                 std::unordered_map<std::string, GLuint> &textureLevel,
                 Spawn                                   &spawn,
                 LevelMetadata                           &metadata);
