#pragma once

struct Wall
{
    float x1, y1, z1;
    float x2, y2, z2;
    int neighbor;
};

struct Sector
{
    int id;
    int startWall;
    int wallCount;
    float floor;
    float ceiling;
};
