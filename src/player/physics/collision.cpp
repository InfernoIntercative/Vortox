#include <cstdio>

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

// glm headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// headers
#include "../../core/globals.hpp"
#include "../../levels/struct.hpp"
#include "../../logsystem/log.hpp"
#include "collision.hpp"

// helper function to check if a value is NaN
bool isNaN(float val) {
    return std::isnan(val); // use standard isnan function
}

// helper function to check if a vector contains NaN
bool containsNaN(const glm::vec3 &v) {
    return isNaN(v.x) || isNaN(v.y) || isNaN(v.z);
}

// debug function to print vector details
void debugVec3(const std::string &label, const glm::vec3 &v) {
    std::cout << label << ": [" << v.x << ", " << v.y << ", " << v.z << "]";
    if (containsNaN(v)) {
        warn("CONTAINS NaN!");
    }
    std::cout << std::endl;
}

bool checkCollision(const glm::vec3 &pos, const std::vector<Wall> &walls,
                    float radius, const glm::vec3 &mapCenter) {
    // basic validation
    if (G_debug) {
        if (walls.empty()) {
            warn("No walls to check collision against");
            return false;
        }
        if (radius <= 0) {
            warn("Invalid collision radius", std::to_string(radius).c_str());
            return false;
        }
    }

    // check for NaN in inputs
    if (G_debug) {
        if (containsNaN(pos)) {
            warn("Player position contains NaN values");
            return false;
        }

        if (containsNaN(mapCenter)) {
            warn("Map center contains NaN values");
            return false;
        }
    }

    // adjust the position into the same coordinate space as the walls.
    glm::vec3 posAdjusted = pos - mapCenter;

    // final validation of adjusted position
    if (G_debug) {
        if (containsNaN(posAdjusted)) {
            warn("Adjusted player position contains NaN values");
            return false;
        }
    }

    // check if player's y position (height) is valid for collision
    float playerHeight = posAdjusted.y;

    for (size_t i = 0; i < walls.size(); ++i) {
        const Wall &wall = walls[i];

        // check for NaN values in wall coordinates
        if (isNaN(wall.x1) || isNaN(wall.y1) || isNaN(wall.z1) || isNaN(wall.x2) ||
            isNaN(wall.y2) || isNaN(wall.z2)) {
            continue; // skip walls with invalid coordinates
        }

        // check if player is within the height range of the wall
        // skip collision if player is above or below the wall
        float minWallHeight = std::min(wall.z1, wall.z2);
        float maxWallHeight = std::max(wall.z1, wall.z2);

        // if player is below the wall's bottom or above the wall's top, no
        // collision
        if (playerHeight + radius < minWallHeight ||
            playerHeight - radius > maxWallHeight) {
            continue;
        }

        // adjust wall endpoints with respect to the map center.
        glm::vec3 A(wall.x1 - mapCenter.x, wall.y1 - mapCenter.y,
                    wall.z1 - mapCenter.z);
        glm::vec3 B(wall.x2 - mapCenter.x, wall.y2 - mapCenter.y,
                    wall.z2 - mapCenter.z);

        // skip walls with NaN coordinates after adjustment
        if (containsNaN(A) || containsNaN(B)) {
            continue;
        }

        // for 2D collision testing, we'll project onto the XZ plane
        glm::vec2 A2D(A.x, A.z);
        glm::vec2 B2D(B.x, B.z);
        glm::vec2 pos2D(posAdjusted.x, posAdjusted.z);

        glm::vec2 AB = B2D - A2D;

        // skip if AB vector is too short
        float ab2 = glm::dot(AB, AB);
        if (ab2 <= std::numeric_limits<float>::epsilon()) {
            continue;
        }

        glm::vec2 AP = pos2D - A2D;

        // calculate projection value
        float t = glm::dot(AP, AB) / ab2;
        t       = glm::clamp(t, 0.0f, 1.0f);

        // find closest point on the wall segment
        glm::vec2 closest = A2D + t * AB;

        // calculate distance to wall
        float distance = glm::length(pos2D - closest);

        // if distance is less than player's radius, there's a collision
        if (distance < radius) {
            return true;
        }
    }

    return false;
}