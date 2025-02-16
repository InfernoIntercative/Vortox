#pragma once

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

void renderText(TTF_Font *font, const std::string &text, float x, float y, GLuint textShaderProgram);
