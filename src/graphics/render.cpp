#include <cstdio>

#include <string>
#include <unordered_map>
#include <vector>

// SDL2
#include <SDL2/SDL_mixer.h>

// OpenGL
#include <GL/glew.h>

// glm
#include <glm/glm.hpp>

// headers
#include "../core/globals.hpp"
#include "../player/input.hpp"

int RT_playMusic(const char *filePath) {
    Mix_Music *music = Mix_LoadMUS(filePath);
    if (!music) {
        fprintf(stderr, "Failed to load music: %s\n", Mix_GetError());
        return -1;
    }
    if (Mix_PlayMusic(music, -1) == -1) {
        fprintf(stderr, "Failed to play music: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        return -1;
    }
    return 0;
}

void RT_render(GLuint shaderProgram, GLint ourTextureLoc,
               std::unordered_map<std::string, GLuint> &textureLevel,
               glm::vec3 &mapCenter, GLint modelLoc, GLint viewLoc,
               GLint projLoc, GLuint VAO, std::vector<float> &levelVertices,
               float M_headbob_offset, std::string R_CurrentTextureKey) {
    // set the clear color (skybox color)
    glClearColor(red, green, blue, alpha);

    // clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniform1i(ourTextureLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    GLuint wallTexture = textureLevel[R_CurrentTextureKey];
    glBindTexture(GL_TEXTURE_2D, wallTexture);

    glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                     glm::vec3(-mapCenter.x, 0.0f, -mapCenter.y));

    // apply head bob offset to the camera's Y position
    glm::vec3 bobbedCameraPos = cameraPos;
    bobbedCameraPos.y += M_headbob_offset;

    glm::mat4 view =
        glm::lookAt(bobbedCameraPos, bobbedCameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(M_fov), static_cast<float>(screen_width) / screen_height,
        0.1f, 100.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, levelVertices.size() / 5);
    glBindVertexArray(0);
}