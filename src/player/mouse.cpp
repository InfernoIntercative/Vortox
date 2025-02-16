#include <SDL2/SDL.h>
#include "input.hpp"
#include "../console/console.hpp"

int input()
{
    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
        if (!consoleActive && event.type == SDL_MOUSEMOTION)
        {
            float xoffset = event.motion.xrel * gMouseSensitivity;
            float yoffset = -event.motion.yrel * gMouseSensitivity;
            yaw += xoffset;
            pitch += yoffset;
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);
        }
    }

    return 0;
}
