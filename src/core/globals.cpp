#include <stdio.h>
#include "globals.hpp"

const float WINDOW_WIDTH = 1360.0f;
const float WINDOW_HEIGHT = 720.0f;
const char WINDOW_TITLE[] = "Xylon Engine (DEVELOPEMENT-NEW-CODE BRANCH)";
const char default_level_path[] = "levels/level.xym";

// player variables
float FOV = 90.0f;

// for other headers or cpp files
float deltaTime;
bool G_Running = false;
