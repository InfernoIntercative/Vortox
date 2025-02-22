#include <stdio.h>
#include <vector>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../console/console.hpp"
#include "keyboard.hpp"
#include "input.hpp"
#include "collision.hpp"

void M_move_forward(const Uint8 *keyStates, const glm::vec3 &forwardDirection,
                    const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                    float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = forwardDirection * M_cameraSpeed * deltaTime;
    float collisionRadius = 0.3f;
    glm::vec3 newPos = cameraPos;

    // Movimento no eixo X
    newPos.x += movement.x;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.x = newPos.x;

    // Movimento no eixo Z
    newPos = cameraPos;
    newPos.z += movement.z;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.z = newPos.z;
}

void M_move_backward(const Uint8 *keyStates, const glm::vec3 &forwardDirection,
                     const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                     float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = -forwardDirection * M_cameraSpeed * deltaTime;
    float collisionRadius = 0.3f;
    glm::vec3 newPos = cameraPos;

    newPos.x += movement.x;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.x = newPos.x;

    newPos = cameraPos;
    newPos.z += movement.z;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.z = newPos.z;
}

void M_move_left(const Uint8 *keyStates, const glm::vec3 &rightDirection,
                 const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                 float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = -rightDirection * M_cameraSpeed * deltaTime;
    float collisionRadius = 0.3f;
    glm::vec3 newPos = cameraPos;

    newPos.x += movement.x;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.x = newPos.x;

    newPos = cameraPos;
    newPos.z += movement.z;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.z = newPos.z;
}

void M_move_right(const Uint8 *keyStates, const glm::vec3 &rightDirection,
                  const std::vector<Wall> &walls, const glm::vec3 &mapCenter,
                  float deltaTime, glm::vec3 &cameraPos)
{
    glm::vec3 movement = rightDirection * M_cameraSpeed * deltaTime;
    float collisionRadius = 0.3f;
    glm::vec3 newPos = cameraPos;

    newPos.x += movement.x;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.x = newPos.x;

    newPos = cameraPos;
    newPos.z += movement.z;
    if (!checkCollision(newPos, walls, collisionRadius, mapCenter))
        cameraPos.z = newPos.z;
}
