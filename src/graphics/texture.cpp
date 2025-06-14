#include <GL/glew.h>
#include <SDL2/SDL_image.h>
#include <cstdio>

#include <iostream>

// Texture loader using SDL_image
GLuint loadTexture(const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        // if the texture fails to load, log the error
        error("IMG_Load failed", IMG_GetError());

        // fallback to missing texture if the main texture fails to load
        surface = IMG_Load("resources/textures/missing.png");
        if (!surface) {
            // if the fallback also fails, log the error and return 0
            panic("Fallback error", IMG_GetError());
            return EXIT_FAILURE;
        }
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // determine format based on surface's bytes per pixel.
    GLint mode = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    // set texture parameters (using GL_REPEAT so the texture tiles)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    SDL_FreeSurface(surface);
    return textureID;
}
