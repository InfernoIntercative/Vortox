#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>

// headers
#include "../logsystem/log.hpp"
#include "audio.hpp"

char lastAudio[255];

void playAudio(const char *filePath, float volume) {
    Mix_Chunk *audio = Mix_LoadWAV(filePath);
    if (!audio) {
        error("Failed to load audio", Mix_GetError());
        return;
    }

    // set volume (SDL_mixer uses values between 0-128)
    // convert from 0.0-1.0 float range to 0-128 int range
    int sdlVolume = static_cast<int>(volume * MIX_MAX_VOLUME);
    Mix_VolumeChunk(audio, sdlVolume);

    // play the audio on the first available channel
    if (Mix_PlayChannel(-1, audio, 0) == -1) {
        error("Failed to play audio", Mix_GetError());
    }

    // copy to "lastAudio" with safety first
    strncpy(lastAudio, filePath, sizeof(lastAudio) - 1);
    lastAudio[sizeof(lastAudio) - 1] = '\0';
}