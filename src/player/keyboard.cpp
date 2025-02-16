#include <stdio.h>

// sdl2
#include <SDL2/SDL.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// headers
#include "../console/console.hpp"
#include "../core/globals.hpp"
#include "keyboard.hpp"

// player headers
#include "input.hpp"
#include "collision.hpp"

void keyboard_input(const std::vector<Wall> &walls, const glm::vec2 &mapCenter)
{
    if (!gPaused && consoleAnim < 0.1f)
    {
        const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
        glm::vec3 forwardDirection = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
        glm::vec3 rightDirection = glm::normalize(glm::cross(forwardDirection, cameraUp));
        glm::vec3 movement(0.0f);
        if (keyStates[SDL_SCANCODE_W])
            movement += forwardDirection;
        if (keyStates[SDL_SCANCODE_S])
            movement -= forwardDirection;
        if (keyStates[SDL_SCANCODE_A])
            movement -= rightDirection;
        if (keyStates[SDL_SCANCODE_D])
            movement += rightDirection;
        if (glm::length(movement) > 0.0f)
            movement = glm::normalize(movement) * cameraSpeed * deltaTime;

        float collisionRadius = 0.3f;

        glm::vec3 newPos = cameraPos;
        newPos.x += movement.x;
        if (!checkCollision(glm::vec2(newPos.x, cameraPos.z), walls, collisionRadius, mapCenter))
            cameraPos.x = newPos.x;

        newPos = cameraPos;
        newPos.z += movement.z;
        if (!checkCollision(glm::vec2(cameraPos.x, newPos.z), walls, collisionRadius, mapCenter))
            cameraPos.z = newPos.z;
    }
}
