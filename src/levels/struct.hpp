#pragma once

#include <iostream>
#include <string>

struct Wall {
    float x1, y1, z1;
    float x2, y2, z2;
    int   neighbor;
};

struct Sector {
    int   id;
    int   startWall;
    int   wallCount;
    float floor;
    float ceiling;
};

struct Spawn {
    float x, y, size;
    float color[3]; // this support rgb
};

struct LevelMetadata {
    std::string version;
    std::string musicPath;
    std::string creator;
    std::string description;
};
