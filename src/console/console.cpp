#include <cstdio>

#include <iostream>
#include <sstream>
#include <unordered_map>

// the boys
#include <string>
#include <vector>

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// OpenGL
#include <GL/glew.h>

// headers
#include "../core/main.hpp"
#include "../logsystem/log.hpp"
#include "../player/input.hpp"
#include "console.hpp"

// levels
#include "../levels/load.hpp"
#include "../levels/struct.hpp"

// important, super important!
bool  G_Paused      = false;
bool  consoleActive = false;
float consoleAnim   = 0.0f;

std::string commandInput     = "";
float       consoleAnimSpeed = 5.0f;
int         consoleHeight    = 300;

void processCommand(const std::string &cmd, bool &G_running,
                    std::vector<Sector> &sectors, std::vector<Wall> &walls,
                    std::vector<float> &levelVertices, GLuint VBO,
                    std::unordered_map<std::string, GLuint> &textureLevel,
                    float &mouseSensitivity, int &currentVolume,
                    SDL_Window               *window,
                    std::vector<std::string> &commandHistory,
                    int                      &historyIndex) {
    std::istringstream iss(cmd);
    std::string        token;
    iss >> token;
    if (token.empty()) {
        commandInput = "";
        return;
    }
    commandHistory.push_back(cmd);
    historyIndex = commandHistory.size();

    if (token == "clear") {
        commandInput = "";
    } else if (token == "exit" || token == "quit") {
        G_running = false;
    } else if (token == "close") {
        consoleActive = false;
    } else if (token == "fly") {
        M_fly        = !M_fly;
        commandInput = "";
    } else if (token == "load") {
        std::string filename;
        iss >> filename;
        if (!filename.empty()) {
            std::vector<Sector>                     newSectors;
            std::vector<Wall>                       newWalls;
            std::unordered_map<std::string, GLuint> newTextureMap;
            LevelMetadata                           metadata;
            if (L_LoadLevel(filename.c_str(), newSectors, newWalls, newTextureMap,
                            playerSpawn, metadata)) {
                sectors       = newSectors;
                walls         = newWalls;
                levelVertices = buildLevelVertices(sectors, walls);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float),
                             levelVertices.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                textureLevel = newTextureMap;
                info("Map loaded", filename.c_str());
            } else {
                warn("Failed to load map!");
            }
        }
        commandInput = "";
    } else if (token == "reload") {
        std::vector<Sector>                     newSectors;
        std::vector<Wall>                       newWalls;
        std::unordered_map<std::string, GLuint> newTextureMap;
        LevelMetadata                           metadata;
        if (L_LoadLevel(L_active_map_path, newSectors, newWalls, newTextureMap,
                        playerSpawn, metadata)) {
            sectors       = newSectors;
            walls         = newWalls;
            levelVertices = buildLevelVertices(sectors, walls);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float),
                         levelVertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            textureLevel = newTextureMap;
            info("Map reloaded", L_active_map_path);
        } else {
            warn("Failed to reload map", L_active_map_path);
        }
        commandInput = "";
    } else if (token == "fullscreen") {
        Uint32 flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_FULLSCREEN)
            SDL_SetWindowFullscreen(window, 0);
        else
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        commandInput = "";
    } else if (token == "pause") {
        G_Paused = !G_Paused;
        std::cout << "Paused: " << (G_Paused ? "true" : "false") << std::endl;
        commandInput = "";
    } else if (token == "sens") {
        float value;
        if (iss >> value) {
            mouseSensitivity = value;
            info("Mouse sensitivity set to",
                 std::to_string(mouseSensitivity).c_str());
        } else {
            error("Invalid sensitivity value", std::to_string(value).c_str());
        }
        commandInput = "";
    } else if (token == "volume") {
        int value;
        if (iss >> value) {
            currentVolume = std::max(0, std::min(128, value));
            Mix_VolumeMusic(currentVolume);
            info("Volume set to", std::to_string(currentVolume).c_str());
        } else {
            error("Invalid volume value", std::to_string(value).c_str());
        }
        commandInput = "";
    } else {
        commandInput = "";
    }
}