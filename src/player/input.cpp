#include <glm/glm.hpp>
#include <stdio.h>
#include "input.hpp"

glm::vec3 cameraPos(0.0f, 2.0f, 9.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float M_cameraSpeed = 10.5f;
float M_yaw = -90.0f;
float M_pitch = 0.0f;

float M_MouseSensitivity = 0.01f;

bool M_fly = true;
