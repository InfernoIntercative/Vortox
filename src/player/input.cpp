#include <glm/glm.hpp>
#include <stdio.h>
#include "input.hpp"

glm::vec3 cameraPos(3.0f, 2.0f, 10.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float cameraSpeed = 10.5f;
float yaw = -90.0f;
float pitch = 0.0f;

float gMouseSensitivity = 0.01f;
