#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../levels/struct.hpp"

void keyboard_input(const std::vector<Wall> &walls, const glm::vec2 &mapCenter);
