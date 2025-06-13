#pragma once

extern const int  screen_width;
extern const int  screen_height;
extern const char window_title[];
extern const char window_icon[];
extern const char build_date[];
extern const bool texture_filtering;

// i don't know
extern const char default_font[];

// maps
extern const char default_level_path[];

// for other headers or cpp files
extern float G_deltaTime;

// TEMP: skybox color will be on the map, not on the engine
extern float red;
extern float green;
extern float blue;
extern float alpha;

extern bool G_running;
extern bool G_debug;
