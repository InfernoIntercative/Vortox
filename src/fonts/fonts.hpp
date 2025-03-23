#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern const char *text_vertex_shader_source;
extern const char *text_fragment_shader_source;

void RT_renderText(TTF_Font *font, const std::string &text, float x, float y,
                   GLuint textShaderProgram);
