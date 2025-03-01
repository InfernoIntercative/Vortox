#include <SDL2/SDL.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// headers
#include "mouse.hpp"

// function to process mouse input and update camera direction
int M_processMouseInput(int dx, int dy, float &M_pitch, float &M_yaw, float M_MouseSensitivity, glm::vec3 &cameraFront)
{
    M_yaw += dx * M_MouseSensitivity;
    M_pitch -= dy * M_MouseSensitivity;

    // clamp pitch
    if (M_pitch > 89.0f)
        M_pitch = 89.0f;
    if (M_pitch < -89.0f)
        M_pitch = -89.0f;

    // update camera front vector
    cameraFront.x = cos(glm::radians(M_yaw)) * cos(glm::radians(M_pitch));
    cameraFront.y = sin(glm::radians(M_pitch));
    cameraFront.z = sin(glm::radians(M_yaw)) * cos(glm::radians(M_pitch));
    cameraFront = glm::normalize(cameraFront);

    return 0;
}
