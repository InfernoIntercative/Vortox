#pragma once

extern const char *textVertexShaderSource;

extern const char *textFragmentShaderSource;

void renderText(TTF_Font *font, const std::string &text, float x, float y, GLuint textShaderProgram);
