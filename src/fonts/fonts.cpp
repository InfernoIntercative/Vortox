#include <stdio.h>
#include <iostream>

// SDL2
#include <SDL2/SDL_ttf.h>

// OpenGL
#include <GL/glew.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// headers
#include "../core/globals.hpp"
#include "fonts.hpp"

const char *textVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
    out vec2 TexCoords;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
    )";

const char *textFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D textTexture;
uniform vec3 textColor;
void main() {
    vec4 sampled = texture(textTexture, TexCoords);
    FragColor = vec4(textColor, 1.0) * sampled;
}
)";

GLuint createTextShaderProgram()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Error compiling text vertex shader: %s\n", infoLog);
        return 0;
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Error compiling text fragment shader: %s\n", infoLog);
        return 0;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Error linking text shader program:\n"
                  << infoLog << std::endl;
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void renderText(TTF_Font *font, const std::string &text, float x, float y, GLuint textShaderProgram)
{
    // enable blending for proper alpha handling
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), white);
    if (!surface)
    {
        std::cerr << "Failed to render text surface: " << TTF_GetError() << std::endl;
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // upload the texture data with an alpha channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    float xpos = x, ypos = y;
    float w = static_cast<float>(surface->w);
    float h = static_cast<float>(surface->h);
    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 1.0f},
        {xpos, ypos, 0.0f, 0.0f},
        {xpos + w, ypos, 1.0f, 0.0f},

        {xpos, ypos + h, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 0.0f},
        {xpos + w, ypos + h, 1.0f, 1.0f}};

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(textShaderProgram);

    // setup an orthographic projection matching your window dimensions.
    glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT), 0.0f);
    GLint projLoc = glGetUniformLocation(textShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(ortho));

    // set text color to white (you can change this to any color)
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);

    // bind the texture and render the quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // cleanup
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(surface);
}
