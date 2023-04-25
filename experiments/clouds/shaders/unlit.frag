#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D Albedo;

void main () {
    FragColor = vec4(texture(Albedo, TexCoord).xyz, 1.0);
}