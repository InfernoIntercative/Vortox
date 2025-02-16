#pragma once

struct Wall
{
    float x1, y1, x2, y2;
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
