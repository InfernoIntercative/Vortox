#include "globals.hpp"

// TEMP: will be non-const
const int  screen_width   = 1280;
const int  screen_height  = 720;
const char window_title[] = "Vortox (test-build)";
const char window_icon[]  = "logo/window.png";
const char build_date[] = "June 11th, 2025";

// console
const char default_font[] = "resources/fonts/console.ttf";

// maps
const char default_level_path[] = "levels/01.dat";
// use bilinear filtering for textures (you can if you don't like it)
const bool texture_filtering = true; // true = bilinear, false = nearest neighbor

// TEMPORARY: skybox color will be on the map, not on the engine
// it's 6/12/2025, and still we don't have a image as skybox, I will start working on it
float red   = 0.0f;
float green = 0.0f;
float blue  = 0.0f;
float alpha = 1.0f;

// for other headers or cpp files
float G_deltaTime;
bool  G_running = false;
bool  G_debug   = false;