#include "input.hpp"

#include <cstdio>

#include <glm/glm.hpp>

// camera position
glm::vec3 cameraPos(0.0f, 2.0f, 9.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float M_cameraSpeed = 7.5f;
float M_yaw         = -90.0f;
float M_pitch       = 0.0f;

float M_collisionRadius = 0.3;

float M_fov = 90.0f;

float M_MouseSensitivity = 0.01f;

bool M_fly              = false;
bool M_enable_collision = true;

// velocity components for acceleration/deceleration
glm::vec3 M_currentVelocity(0.0f, 0.0f, 0.0f);
float     M_acceleration = 30.0f; // units per second squared
float     M_deceleration = 40.0f; // units per second squared
float     M_maxVelocity =
    10.5f; // maximum velocity (same as original camera speed)