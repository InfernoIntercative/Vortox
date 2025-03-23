#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

int M_processMouseInput(int dx, int dy, float &M_pitch, float &M_yaw, float M_MouseSensitivity, glm::vec3 &cameraFront);
