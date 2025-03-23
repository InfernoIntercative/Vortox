#include "globals.hpp"

// TEMP: will be non-const
const int  screen_width   = 1360;
const int  screen_height  = 720;
const char window_title[] = "Vortox Engine (DEVELOPEMENT-NEW-CODE BRANCH)";
const char window_icon[]  = "logo/window.png";

// i don't know
const char default_font[] = "resources/fonts/console.ttf";

// maps
const char default_level_path[] = "levels/d1.xym";

// TEMP: skybox color will be on the map, not on the engine
float red   = 0.0f;
float green = 0.0f;
float blue  = 0.0f;
float alpha = 1.0f;

// for other headers or cpp files
float G_deltaTime;
bool  G_running = false;
bool  G_debug   = false;