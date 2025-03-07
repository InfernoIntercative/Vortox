#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../levels/struct.hpp"

void moveWithCollision(glm::vec3 &cameraPos, const glm::vec3 &movement,
                       const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                       float collisionRadius);

void M_move_forward(const glm::vec3 &forwardDirection,
                    const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                    float dt, glm::vec3 &cameraPos);

void M_move_backward(const glm::vec3 &forwardDirection,
                     const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                     float dt, glm::vec3 &cameraPos);

void M_move_left(const glm::vec3 &rightDirection,
                 const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                 float dt, glm::vec3 &cameraPos);

void M_move_right(const glm::vec3 &rightDirection,
                  const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                  float dt, glm::vec3 &cameraPos);
