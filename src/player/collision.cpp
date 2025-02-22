#include <stdio.h>
#include <vector>
#include "collision.hpp"
#include "../levels/struct.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool checkCollision(const glm::vec3 &pos, const std::vector<Wall> &walls, float radius, const glm::vec3 &mapCenter)
{
    for (const auto &wall : walls)
    {
        // Ajusta as coordenadas com base no centro do mapa
        glm::vec3 A(wall.x1 - mapCenter.x, wall.y1 - mapCenter.y, wall.z1 - mapCenter.z);
        glm::vec3 B(wall.x2 - mapCenter.x, wall.y2 - mapCenter.y, wall.z2 - mapCenter.z);

        glm::vec3 AB = B - A;
        float ab2 = glm::dot(AB, AB);
        if (ab2 == 0.0f)
            continue;

        glm::vec3 AP = pos - A;
        float t = glm::dot(AP, AB) / ab2;
        t = glm::clamp(t, 0.0f, 1.0f);

        glm::vec3 closest = A + t * AB;

        if (glm::length(pos - closest) < radius)
            return true;
    }
    return false;
}
