#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../levels/struct.hpp"

bool checkCollision(const glm::vec2 &pos, const std::vector<Wall> &walls, float radius, const glm::vec2 &mapCenter);
