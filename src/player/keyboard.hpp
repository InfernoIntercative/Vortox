#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../levels/struct.hpp"

// Updates velocity in the given direction based on acceleration/deceleration
void updateVelocity(glm::vec3 &velocity, const glm::vec3 &direction,
                    bool isMoving, float deltaTime);

void moveWithCollision(glm::vec3 &cameraPos, const glm::vec3 &movement,
                       const std::vector<Wall> &walls,
                       const glm::vec3 &mapCenter, float collisionRadius);

void M_move_forward(const glm::vec3         &forwardDirection,
                    const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                    float G_deltaTime, glm::vec3 &cameraPos);

void M_move_backward(const glm::vec3         &forwardDirection,
                     const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                     float G_deltaTime, glm::vec3 &cameraPos);

void M_move_left(const glm::vec3         &rightDirection,
                 const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                 float G_deltaTime, glm::vec3 &cameraPos);

void M_move_right(const glm::vec3         &rightDirection,
                  const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                  float G_deltaTime, glm::vec3 &cameraPos);

// Add deceleration when no movement keys are pressed
void M_apply_deceleration(const std::vector<Wall> &walls,
                          const glm::vec3 &mapCenter, float G_deltaTime,
                          glm::vec3 &cameraPos);
