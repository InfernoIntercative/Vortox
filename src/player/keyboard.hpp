#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../levels/struct.hpp"

void moveWithCollision(glm::vec3 &cameraPos, const glm::vec3 &movement,
                       const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                       float collisionRadius);

void M_move_forward(const Uint8 *keyStates, const glm::vec3 &forwardDirection,
                    const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                    float deltaTime, glm::vec3 &cameraPos);

void M_move_backward(const Uint8 *keyStates, const glm::vec3 &forwardDirection,
                     const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                     float deltaTime, glm::vec3 &cameraPos);

void M_move_left(const Uint8 *keyStates, const glm::vec3 &rightDirection,
                 const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                 float deltaTime, glm::vec3 &cameraPos);

void M_move_right(const Uint8 *keyStates, const glm::vec3 &rightDirection,
                  const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                  float deltaTime, glm::vec3 &cameraPos);
