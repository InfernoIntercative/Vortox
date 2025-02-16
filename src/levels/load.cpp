#include <stdio.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "struct.hpp"
#include "../graphics/texture.hpp"

// --- modified loadMap function that also loads textures ---
bool loadMap(const char *filename, std::vector<Sector> &sectors, std::vector<Wall> &walls,
             std::unordered_map<std::string, GLuint> &textureLevel)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open map: " << filename << std::endl;
        return false;
    }
    std::string line;
    enum Section
    {
        NONE,
        SECTOR,
        WALL,
        TEXTURES
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
            // expect a line of the form: key=path/to/texture.png
            size_t eqPos = token.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = token.substr(0, eqPos);
                std::string path = token.substr(eqPos + 1);
                // append any remaining tokens (in case the path contains spaces)
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
    }
    return true;
}
