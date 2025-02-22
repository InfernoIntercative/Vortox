#include <stdio.h>
#include <vector>     // Make sure this is here
#include <SDL2/SDL.h> // Defines Uint8
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../console/console.hpp"
#include "keyboard.hpp"
#include "input.hpp"
#include "collision.hpp"

float collisionRadius = 0.3;

// helper function to perform movement with collision detection.
void moveWithCollision(glm::vec3 &cameraPos, const glm::vec3 &movement,
                       const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                       float collisionRadius)
{
    // try moving along the full movement vector.
    glm::vec3 newPos = cameraPos + movement;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
    {
        cameraPos = newPos;
    }
    else
    {
        // if the full movement results in a collision, try axis-separated movement.
        glm::vec3 testPos = cameraPos;
        testPos.x += movement.x;
        if (!checkCollision(testPos, walls, collisionRadius, mapCenter))
            cameraPos.x += movement.x;

        testPos = cameraPos;
        testPos.z += movement.z;
        if (!checkCollision(testPos, walls, collisionRadius, mapCenter))
            cameraPos.z += movement.z;
    }
}

void M_move_forward(const Uint8 *keyStates, const glm::vec3 &forwardDirection,
                    const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                    float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = forwardDirection * M_cameraSpeed * deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, collisionRadius);
}

void M_move_backward(const Uint8 *keyStates, const glm::vec3 &forwardDirection,
                     const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                     float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = -forwardDirection * M_cameraSpeed * deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, collisionRadius);
}

void M_move_left(const Uint8 *keyStates, const glm::vec3 &rightDirection,
                 const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                 float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = -rightDirection * M_cameraSpeed * deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, collisionRadius);
}

void M_move_right(const Uint8 *keyStates, const glm::vec3 &rightDirection,
                  const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                  float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = rightDirection * M_cameraSpeed * deltaTime;
    moveWithCollision(cameraPos, movement, walls, mapCenter, collisionRadius);
}
