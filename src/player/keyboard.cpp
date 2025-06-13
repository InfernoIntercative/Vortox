#include "keyboard.hpp"

#include <SDL2/SDL.h>
#include <cstdio>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "../core/globals.hpp"
#include "../logsystem/log.hpp"
#include "input.hpp"
#include "physics/collision.hpp"

// Updates velocity in the given direction based on acceleration/deceleration
void updateVelocity(glm::vec3 &velocity, const glm::vec3 &direction,
                    bool isMoving, float deltaTime) {
    if (isMoving) {
        // Accelerate in the given direction
        glm::vec3 targetVelocity = direction * M_maxVelocity;
        glm::vec3 acceleration =
            (targetVelocity - velocity) * M_acceleration * deltaTime;

        // Limit acceleration magnitude
        float accelerationMagnitude = glm::length(acceleration);
        if (accelerationMagnitude > M_acceleration * deltaTime) {
            acceleration = glm::normalize(acceleration) * M_acceleration * deltaTime;
        }

        velocity += acceleration;

        // Limit velocity to maximum speed
        float speed = glm::length(velocity);
        if (speed > M_maxVelocity) {
            velocity = glm::normalize(velocity) * M_maxVelocity;
        }
    } else {
        // Decelerate when not moving in the given direction
        float speed = glm::length(velocity);
        if (speed > 0.01f) {
            // Calculate a deceleration factor that decreases as speed decreases
            // This creates a more natural deceleration curve
            float     decelFactor = glm::mix(0.5f, 1.0f, glm::min(1.0f, speed / M_maxVelocity));
            glm::vec3 deceleration =
                glm::normalize(velocity) * M_deceleration * decelFactor * deltaTime;

            // Make sure we don't decelerate past zero
            if (glm::length(deceleration) > speed) {
                velocity = glm::vec3(0.0f);
            } else {
                velocity -= deceleration;
            }
        } else {
            // Zero out very small values to avoid floating point issues
            velocity = glm::vec3(0.0f);
        }
    }
}

void moveWithCollision(glm::vec3 &cameraPos, const glm::vec3 &movement,
                       const std::vector<Wall> &walls,
                       const glm::vec3 &mapCenter, float collisionRadius) {
    static int debugCounter = 0;

    // only print debug info every 100 frames to avoid console spam
    bool shouldPrintDebug = (debugCounter++ % 100 == 0);

    if (shouldPrintDebug) {
        if (G_debug) {
            std::cout << "--- Collision Move Debug ---" << std::endl;
            std::cout << "Current position: [" << cameraPos.x << ", " << cameraPos.y
                      << ", " << cameraPos.z << "]" << std::endl;
            std::cout << "Attempted movement: [" << movement.x << ", " << movement.y
                      << ", " << movement.z << "]" << std::endl;
            std::cout << "Map center: [" << mapCenter.x << ", " << mapCenter.y << ", "
                      << mapCenter.z << "]" << std::endl;
            std::cout << "Collision radius: " << collisionRadius << std::endl;
        }
    }

    // check for NaN in inputs
    if (std::isnan(cameraPos.x) || std::isnan(cameraPos.y) ||
        std::isnan(cameraPos.z) || std::isnan(movement.x) ||
        std::isnan(movement.y) || std::isnan(movement.z) ||
        std::isnan(mapCenter.x) || std::isnan(mapCenter.y) ||
        std::isnan(mapCenter.z) || std::isnan(collisionRadius)) {
        if (G_debug) {
            warn(
                "Warning: NaN detected in collision parameters - movement cancelled");
        }
        return;
    }

    // try moving along the full movement vector.
    glm::vec3 newPos = cameraPos + movement;

    if (shouldPrintDebug) {
        if (G_debug) {
            // not using info() because it's not a string
            std::cout << "Target position: [" << newPos.x << ", " << newPos.y << ", "
                      << newPos.z << "]" << std::endl;
        }
    }

    bool collision = checkCollision(newPos, walls, collisionRadius, mapCenter);

    if (!collision) {
        if (shouldPrintDebug) {
            if (G_debug) {
                info("No collision detected, moving to target position");
            }
        }
        cameraPos = newPos;
    } else {
        if (shouldPrintDebug) {
            if (G_debug) {
                info("Collision detected, trying axis-aligned movement");
            }
        }

        // try moving along X axis first.
        glm::vec3 testPos = cameraPos;
        testPos.x += movement.x;

        if (shouldPrintDebug) {
            if (G_debug) {
                // not using info() because it's not a string
                std::cout << "Trying X-axis movement: [" << testPos.x << ", "
                          << testPos.y << ", " << testPos.z << "]" << std::endl;
            }
        }

        if (!checkCollision(testPos, walls, collisionRadius, mapCenter)) {
            if (shouldPrintDebug) {
                if (G_debug) {
                    info("X-axis movement succeeded");
                }
            }
            cameraPos.x += movement.x;
        }

        // then try moving along Z axis.
        testPos = cameraPos;
        testPos.z += movement.z;

        if (shouldPrintDebug) {
            if (G_debug) {
                // not using info() because it's not a string
                std::cout << "Trying Z-axis movement: [" << testPos.x << ", "
                          << testPos.y << ", " << testPos.z << "]" << std::endl;
            }
        }

        if (!checkCollision(testPos, walls, collisionRadius, mapCenter)) {
            if (shouldPrintDebug) {
                if (G_debug) {
                    info("Z-axis movement succeeded");
                }
            }
            cameraPos.z += movement.z;
        }
    }

    if (shouldPrintDebug) {
        if (G_debug) {
            // not using info() because it's not a string
            std::cout << "Final position: [" << cameraPos.x << ", " << cameraPos.y
                      << ", " << cameraPos.z << "]" << std::endl;
        }
    }
}

void M_move_forward(const glm::vec3         &forwardDirection,
                    const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                    float G_deltaTime, glm::vec3 &cameraPos) {
    // update velocity in the forward direction
    updateVelocity(M_currentVelocity, forwardDirection, true, G_deltaTime);

    // apply movement using the current velocity
    glm::vec3 movement = M_currentVelocity * G_deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, M_collisionRadius);
}

void M_move_backward(const glm::vec3         &forwardDirection,
                     const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                     float G_deltaTime, glm::vec3 &cameraPos) {
    // update velocity in the backward direction
    updateVelocity(M_currentVelocity, -forwardDirection, true, G_deltaTime);

    // apply movement using the current velocity
    glm::vec3 movement = M_currentVelocity * G_deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, M_collisionRadius);
}

void M_move_left(const glm::vec3         &rightDirection,
                 const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                 float G_deltaTime, glm::vec3 &cameraPos) {
    // update velocity in the left direction
    updateVelocity(M_currentVelocity, -rightDirection, true, G_deltaTime);

    // apply movement using the current velocity
    glm::vec3 movement = M_currentVelocity * G_deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, M_collisionRadius);
}

void M_move_right(const glm::vec3         &rightDirection,
                  const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                  float G_deltaTime, glm::vec3 &cameraPos) {
    // update velocity in the right direction
    updateVelocity(M_currentVelocity, rightDirection, true, G_deltaTime);

    // apply movement using the current velocity
    glm::vec3 movement = M_currentVelocity * G_deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, M_collisionRadius);
}

// add deceleration when no movement keys are pressed
void M_apply_deceleration(const std::vector<Wall> &walls,
                          const glm::vec3 &mapCenter, float G_deltaTime,
                          glm::vec3 &cameraPos) {
    // apply deceleration when no movement key is pressed
    updateVelocity(M_currentVelocity, glm::vec3(0.0f), false, G_deltaTime);

    // apply movement using the decelerating velocity
    if (glm::length(M_currentVelocity) > 0.01f) {
        glm::vec3 movement = M_currentVelocity * G_deltaTime;
        moveWithCollision(cameraPos, movement, walls, mapCenter, M_collisionRadius);
    }
}
