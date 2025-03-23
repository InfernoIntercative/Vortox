#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D textTexture;
uniform vec3 textColor;

void main() {
    // Simply sample the texture
    vec4 sampled = texture(textTexture, TexCoords);
    
    // Basic alpha testing
    if (sampled.a < 0.05)
        discard;
    
    // Just output the color with the sampled alpha
    FragColor = vec4(textColor.rgb, sampled.a);
}