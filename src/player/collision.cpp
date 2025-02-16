#include <stdio.h>
#include <vector>

#include "collision.hpp"
#include "../levels/struct.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool checkCollision(const glm::vec2 &pos, const std::vector<Wall> &walls, float radius, const glm::vec2 &mapCenter)
{
    for (const auto &wall : walls)
    {
        glm::vec2 A(wall.x1 - mapCenter.x, wall.y1 - mapCenter.y);
        glm::vec2 B(wall.x2 - mapCenter.x, wall.y2 - mapCenter.y);
        glm::vec2 AB = B - A;
        float ab2 = glm::dot(AB, AB);
        if (ab2 == 0.0f)
            continue;
        glm::vec2 AP = pos - A;
        float t = glm::dot(AP, AB) / ab2;
        t = glm::clamp(t, 0.0f, 1.0f);
        glm::vec2 closest = A + t * AB;
        if (glm::length(pos - closest) < radius)
            return true;
    }
    return false;
}
