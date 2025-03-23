#include <SDL2/SDL.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// headers
#include "mouse.hpp"

// PLEASE! I BEG YOU!
// DO NOT CHANGE THIS VALUE!
// IF THE VALUE IS 0.0F, THE PLAYER WILL BE ABLE TO LOOK AROUND 360 DEGREES
float M_clamp_pitch = 0.0f;

// function to process mouse input and update camera direction
int M_processMouseInput(int dx, int dy, float &M_pitch, float &M_yaw,
                        float M_MouseSensitivity, glm::vec3 &cameraFront) {
    M_yaw += dx * M_MouseSensitivity;
    M_pitch -= dy * M_MouseSensitivity;

    // clamp pitch (0.0f = invalid)
    if (M_clamp_pitch != 0.0f) {
        if (M_pitch > M_clamp_pitch)
            M_pitch = M_clamp_pitch;
        if (M_pitch < -M_clamp_pitch)
            M_pitch = -M_clamp_pitch;
    }

    // update camera front vector
    cameraFront.x = cos(glm::radians(M_yaw)) * cos(glm::radians(M_pitch));
    cameraFront.y = sin(glm::radians(M_pitch));
    cameraFront.z = sin(glm::radians(M_yaw)) * cos(glm::radians(M_pitch));
    cameraFront   = glm::normalize(cameraFront);

    return 0;
}
