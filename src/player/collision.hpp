#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../levels/struct.hpp"

bool checkCollision(const glm::vec3 &pos, const std::vector<Wall> &walls, float radius, const glm::vec3 &mapCenter);
