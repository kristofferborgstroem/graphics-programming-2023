#version 330 core

layout (location = 0) in vec3 WorldPosition;
layout (location = 1) in vec2 WorldTexture;

out vec2 TexCoord;

void main() {
    TexCoord = WorldTexture;
    gl_Position = vec4(WorldPosition, 1.0);
}