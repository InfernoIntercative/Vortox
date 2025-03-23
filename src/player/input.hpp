#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;

extern float M_cameraSpeed;
extern float M_jumpSpeed;
extern float M_gravity;
extern float M_yaw;
extern float M_pitch;
extern float M_collisionRadius;

extern float M_fov;

extern float M_MouseSensitivity;

extern bool M_fly;
extern bool M_enable_collision;

// Velocity components for acceleration/deceleration
extern glm::vec3 M_currentVelocity;
extern float     M_acceleration;
extern float     M_deceleration;
extern float     M_maxVelocity;
