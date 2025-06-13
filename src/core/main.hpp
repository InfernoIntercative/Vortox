#pragma once

#include <string>
#include <vector>

// headers
#include "../levels/struct.hpp"

// player spawn data
extern Spawn playerSpawn;

std::vector<float> buildLevelVertices(const std::vector<Sector> &sectors, const std::vector<Wall> &walls);