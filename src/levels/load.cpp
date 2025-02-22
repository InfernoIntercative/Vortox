#include <stdio.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "struct.hpp"

// headers
#include "../graphics/texture.hpp"
#include "../errors/error.hpp"

// -- load the level com spawn --
bool L_loadLevel(const char *filename, std::vector<Sector> &sectors, std::vector<Wall> &walls,
                 std::unordered_map<std::string, GLuint> &textureLevel, Spawn &spawn)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        critical("Failed to open map", filename);
    }
    std::string line;
    enum Section
    {
        NONE,
        SECTOR,
        WALL,
        TEXTURES,
        SPAWN
    };
    Section currentSection = NONE;
    while (std::getline(file, line))
    {
        // remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        std::istringstream linestream(line);
        std::string token;
        if (!(linestream >> token))
            continue;

        if (token == "[SECTOR]")
        {
            currentSection = SECTOR;
            continue;
        }
        else if (token == "[WALL]")
        {
            currentSection = WALL;
            continue;
        }
        else if (token == "[TEXTURES]")
        {
            currentSection = TEXTURES;
            continue;
        }
        else if (token == "[SPAWN]")
        {
            currentSection = SPAWN;
            continue;
        }
        else if (token[0] == '[')
        {
            currentSection = NONE;
            continue;
        }

        if (currentSection == SECTOR)
        {
            Sector sector;
            sector.id = std::stoi(token);
            linestream >> sector.startWall >> sector.wallCount >> sector.floor >> sector.ceiling;
            sectors.push_back(sector);
        }
        else if (currentSection == WALL)
        {
            Wall wall;
            wall.x1 = std::stof(token);
            linestream >> wall.y1 >> wall.x2 >> wall.y2 >> wall.neighbor;
            walls.push_back(wall);
        }
        else if (currentSection == TEXTURES)
        {
            // espera uma linha no formato: key=path/to/texture.png
            size_t eqPos = token.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = token.substr(0, eqPos);
                std::string path = token.substr(eqPos + 1);
                // concatena tokens adicionais (caso o path tenha espaços)
                std::string extra;
                while (linestream >> extra)
                    path += " " + extra;
                GLuint textureID = loadTexture(path.c_str());
                if (textureID == 0)
                {
                    std::cerr << "Failed to load texture for key: " << key << " from path: " << path << std::endl;
                }
                textureLevel[key] = textureID;
            }
        }
        else if (currentSection == SPAWN)
        {
            // espera linhas do tipo key=value
            size_t eqPos = token.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = token.substr(0, eqPos);
                if (key == "x")
                {
                    std::string valueStr = token.substr(eqPos + 1);
                    float xVal;
                    if (valueStr.empty())
                        linestream >> xVal;
                    else
                        xVal = std::stof(valueStr);
                    spawn.x = xVal;
                }
                else if (key == "y")
                {
                    std::string valueStr = token.substr(eqPos + 1);
                    float yVal;
                    if (valueStr.empty())
                        linestream >> yVal;
                    else
                        yVal = std::stof(valueStr);
                    spawn.y = yVal;
                }
                else if (key == "size")
                {
                    std::string valueStr = token.substr(eqPos + 1);
                    float sVal;
                    if (valueStr.empty())
                        linestream >> sVal;
                    else
                        sVal = std::stof(valueStr);
                    spawn.size = sVal;
                }
                else if (key == "color")
                {
                    // espera três floats: r, g, b
                    std::string valueStr = token.substr(eqPos + 1);
                    float r, g, b;
                    if (valueStr.empty())
                    {
                        linestream >> r >> g >> b;
                    }
                    else
                    {
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
