#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>

// headers
#include "../logsystem/log.hpp"
#include "audio.hpp"

char lastAudio[255];
unsigned char log_of_all_audio[65535];
size_t log_audio_offset = 0;
// lastChannel keeps track of the last played audio channel
int lastChannel = -1;

void playAudio(const char *filePath, float volume) {
    // stop previous audio if still playing
    if (lastChannel != -1) {
        Mix_HaltChannel(lastChannel);
        lastChannel = -1;
    }

    Mix_Chunk *audio = Mix_LoadWAV(filePath);
    if (!audio) {
        error("Failed to load audio", Mix_GetError());
        return;
    }

    int sdlVolume = static_cast<int>(volume * MIX_MAX_VOLUME);
    Mix_VolumeChunk(audio, sdlVolume);

    // play the audio on the first available channel and store the channel
    lastChannel = Mix_PlayChannel(-1, audio, 0);
    if (lastChannel == -1) {
        error("Failed to play audio", Mix_GetError());
        Mix_FreeChunk(audio);
        return;
    }

    // copy to "lastAudio" with safety first
    strncpy(lastAudio, filePath, sizeof(lastAudio) - 1);
    lastAudio[sizeof(lastAudio) - 1] = '\0';

    // log the audio file path
    logAudioFile(filePath);

    // free the chunk after playback is done (using a callback)
    auto freeChunk = [](int channel) {
        Mix_Chunk *finishedChunk = Mix_GetChunk(channel);
        if (finishedChunk) {
            Mix_FreeChunk(finishedChunk);
        }
    };
    Mix_ChannelFinished(freeChunk);
}

// helper function to log audio file paths
void logAudioFile(const char* filePath) {
    size_t len = strlen(filePath) + 1; // include null terminator
    if (log_audio_offset + len < sizeof(log_of_all_audio)) {
        memcpy(&log_of_all_audio[log_audio_offset], filePath, len);
        log_audio_offset += len;
    }
    // else: log is full, ignore new entries
}