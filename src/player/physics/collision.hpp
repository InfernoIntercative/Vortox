#pragma once

#include <glm/glm.hpp>
#include <vector>

// headers
#include "../../levels/struct.hpp"

bool isNaN(float val);
bool containsNaN(const glm::vec3 &v);
void debugVec3(const std::string &label, const glm::vec3 &v);
bool checkCollision(const glm::vec3 &pos, const std::vector<Wall> &walls,
                    float radius, const glm::vec3 &mapCenter);
