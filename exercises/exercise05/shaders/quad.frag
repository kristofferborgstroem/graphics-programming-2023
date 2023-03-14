#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D DepthMap;

void main() {
    FragColor = vec4(vec3(texture(DepthMap, TexCoord).r), 1.0);
}