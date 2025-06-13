#pragma once

int RT_playMusic(const char *filePath);

void RT_render(GLuint shaderProgram, GLint ourTextureLoc,
               std::unordered_map<std::string, GLuint> &textureLevel,
               glm::vec3 &mapCenter, GLint modelLoc, GLint viewLoc, GLint projLoc,
               GLuint VAO, std::vector<float> &levelVertices, float M_headbob_offset, std::string R_CurrentTextureKey);