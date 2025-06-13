#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>
#include <string>

class SDLContext {
public:
    SDLContext(Uint32 sdl_flags = SDL_INIT_VIDEO, int img_flags = IMG_INIT_PNG) {
        if (SDL_Init(sdl_flags) != 0) {
            throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }
        if (!(IMG_Init(img_flags) & img_flags)) {
            throw std::runtime_error("IMG_Init failed: " + std::string(IMG_GetError()));
        }
        if (TTF_Init() != 0) {
            throw std::runtime_error("TTF_Init failed: " + std::string(TTF_GetError()));
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
            throw std::runtime_error("Mix_OpenAudio failed: " + std::string(Mix_GetError()));
        }
    }

    ~SDLContext() {
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    SDLContext(const SDLContext &) = delete;
    SDLContext &operator=(const SDLContext &) = delete;
    SDLContext(SDLContext &&) = delete;
    SDLContext &operator=(SDLContext &&) = delete;
};