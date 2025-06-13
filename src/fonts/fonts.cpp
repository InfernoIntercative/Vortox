#include <cstdio>

#include <iostream>

// headers
#include "../core/globals.hpp"
#include "../logsystem/log.hpp"
#include "../shaders/shaders.hpp"
#include "fonts.hpp"

void RT_renderText(TTF_Font *font, const std::string &text, float x, float y,
                   GLuint textShaderProgram) {
    if (text.empty()) {
        return; // don't try to render empty text
    }

    // Create a new surface for every text rendering - no state leakage
    SDL_Color    white   = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), white);
    if (!surface) {
        warn("Failure to render text surface", TTF_GetError());
        return;
    }

    // simple texture setup - nothing fancy
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // use standard GL_RGBA format with 8 bits per channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, surface->pixels);

    // simple linear filtering
    if (texture_filtering) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    // setup vertices for the quad
    float xpos = x;
    float ypos = y;
    float w    = static_cast<float>(surface->w);
    float h    = static_cast<float>(surface->h);

    // simple quad with texture coordinates
    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 1.0f},
        {xpos, ypos, 0.0f, 0.0f},
        {xpos + w, ypos, 1.0f, 0.0f},

        {xpos, ypos + h, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 0.0f},
        {xpos + w, ypos + h, 1.0f, 1.0f}};

    // create VBO and VAO for rendering
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // simple vao setup
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // use the shader program
    glUseProgram(textShaderProgram);

    // setup orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screen_width),
                                      static_cast<float>(screen_height), 0.0f);
    GLint     projLoc    = glGetUniformLocation(textShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // set white text color
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);

    // setup texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // disable depth testing for text
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }

    // draw the text
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(surface);

    // restore depth testing if it was enabled
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}