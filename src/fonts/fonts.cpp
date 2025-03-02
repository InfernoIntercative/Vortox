#include <iostream>
#include <stdio.h>

// headers
#include "fonts.hpp"
#include "../core/globals.hpp"
#include "../shaders/shaders.hpp"
#include "../errors/error.hpp"

// vertex Shader
const char *text_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

// fragment Shader
const char *text_fragment_shader_source = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D textTexture;
uniform vec3 textColor;
void main() {
    vec4 sampled = texture(textTexture, TexCoords);
    FragColor = vec4(textColor, sampled.a);
}
)";

void renderText(TTF_Font *font, const std::string &text, float x, float y, GLuint textShaderProgram) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), white);
    if (!surface) {
        warn("Failure to render text surface", TTF_GetError());
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    float xpos = x;
    float ypos = y;
    float w = static_cast<float>(surface->w);
    float h = static_cast<float>(surface->h);
    float vertices[6][4] = {
        {xpos,     ypos + h, 0.0f, 1.0f},
        {xpos,     ypos,     0.0f, 0.0f},
        {xpos + w, ypos,     1.0f, 0.0f},

        {xpos,     ypos + h, 0.0f, 1.0f},
        {xpos + w, ypos,     1.0f, 0.0f},
        {xpos + w, ypos + h, 1.0f, 1.0f}
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(textShaderProgram);

    glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH),
                                 static_cast<float>(WINDOW_HEIGHT), 0.0f);
    GLint projLoc = glGetUniformLocation(textShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(ortho));

    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // clean up clean up clean up
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(surface);
}
