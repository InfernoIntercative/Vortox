#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

// OpenGL
#include <GL/glew.h>

#include <fstream>
#include <iostream>
#include <sstream>

// headers
#include "../graphics/texture.hpp"
#include "../logsystem/log.hpp"
#include "struct.hpp"

char L_active_map_path[]; // before it was active_map_path

// load the level com spawn
bool L_LoadLevel(const char *filename, std::vector<Sector> &sectors,
                 std::vector<Wall>                       &walls,
                 std::unordered_map<std::string, GLuint> &textureLevel,
                 Spawn                                   &spawn,
                 LevelMetadata                           &metadata) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        panic("Failed to open map", filename);
    }

    // copy level path to L_active_map_path, and probably will explode the code
    // (me in feature) i fixed
    strncpy(L_active_map_path, filename, sizeof(L_active_map_path) - 1);
    L_active_map_path[sizeof(L_active_map_path) - 1] = '\0'; // ensure null-termination

    std::string line;
    enum Section {
        NONE,
        SECTOR,
        WALL,
        TEXTURES,
        SPAWN,
        METADATA
    };
    Section currentSection  = NONE;
    bool    inMetadataBlock = false;

    while (std::getline(file, line)) {
        // remove comments
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        std::istringstream linestream(line);
        std::string        token;
        if (!(linestream >> token))
            continue;

        // check for metadata block
        if (token == "{") {
            inMetadataBlock = true;
            continue;
        } else if (token == "}") {
            inMetadataBlock = false;
            continue;
        }

        // check for level_version outside of metadata block
        size_t eqPos = token.find('=');
        if (eqPos != std::string::npos && token.substr(0, eqPos) == "level_version") {
            metadata.version = token.substr(eqPos + 1);
            // remove any whitespace
            metadata.version.erase(0, metadata.version.find_first_not_of(" \t"));
            continue;
        }

        // handle metadata within the block
        if (inMetadataBlock && eqPos != std::string::npos) {
            std::string key   = token.substr(0, eqPos);
            std::string value = token.substr(eqPos + 1);

            // remove any whitespace
            value.erase(0, value.find_first_not_of(" \t"));

            // remove quotes if present
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            if (key == "musicpath") {
                metadata.musicPath = value;
                if (G_debug) {
                    debug("Found musicpath", value.c_str());
                }
            } else if (key == "creator") {
                metadata.creator = value;
                if (G_debug) {
                    debug("Found creator", value.c_str());
                }
            } else if (key == "description") {
                metadata.description = value;
                if (G_debug) {
                    debug("Found description", value.c_str());
                }
            }

            continue;
        }

        // section
        if (token == "[SECTOR]") {
            currentSection = SECTOR;
            continue;
        } else if (token == "[WALL]") {
            currentSection = WALL;
            continue;
        } else if (token == "[TEXTURES]") {
            currentSection = TEXTURES;
            continue;
        } else if (token == "[SPAWN]") {
            currentSection = SPAWN;
            continue;
        } else if (token[0] == '[') {
            currentSection = NONE;
            continue;
        }
        if (currentSection == SECTOR) {
            Sector sector;
            sector.id = std::stoi(token);
            linestream >> sector.startWall >> sector.wallCount >> sector.floor >>
                sector.ceiling;
            sectors.push_back(sector);
        } else if (currentSection == WALL) {
            Wall wall;
            wall.x1 = std::stof(token);
            linestream >> wall.y1 >> wall.x2 >> wall.y2 >> wall.neighbor;
            walls.push_back(wall);
        } else if (currentSection == TEXTURES) {
            // example: key=path/to/texture.png
            size_t eqPos = token.find('=');
            if (eqPos != std::string::npos) {
                std::string key  = token.substr(0, eqPos);
                std::string path = token.substr(eqPos + 1);
                std::string extra;

                while (linestream >> extra)
                    path += " " + extra;
                GLuint textureID = loadTexture(path.c_str());
                if (textureID == 0) {
                    warn("Failed to load texture for key", key.c_str());
                }
                textureLevel[key] = textureID;
            }
        } else if (currentSection == SPAWN) {
            size_t eqPos = token.find('=');
            if (eqPos != std::string::npos) {
                std::string key = token.substr(0, eqPos);
                if (key == "x") {
                    std::string valueStr = token.substr(eqPos + 1);
                    float       xVal;
                    if (valueStr.empty())
                        linestream >> xVal;
                    else
                        xVal = std::stof(valueStr);
                    spawn.x = xVal;
                } else if (key == "y") {
                    std::string valueStr = token.substr(eqPos + 1);
                    float       yVal;
                    if (valueStr.empty())
                        linestream >> yVal;
                    else
                        yVal = std::stof(valueStr);
                    spawn.y = yVal;
                } else if (key == "size") {
                    std::string valueStr = token.substr(eqPos + 1);
                    float       sVal;
                    if (valueStr.empty())
                        linestream >> sVal;
                    else
                        sVal = std::stof(valueStr);
                    spawn.size = sVal;
                } else if (key == "color") {
                    // 3 floats, R G B
                    std::string valueStr = token.substr(eqPos + 1);
                    float       r, g, b;
                    if (valueStr.empty()) {
                        linestream >> r >> g >> b;
                    } else {
                        r = std::stof(valueStr);
                        linestream >> g >> b;
                    }
                    spawn.color[0] = r;
                    spawn.color[1] = g;
                    spawn.color[2] = b;
                }
            }
        }
    }
    return true;
}
