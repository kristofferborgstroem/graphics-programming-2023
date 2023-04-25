#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 TexturePosition;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjectionMatrix;

void main () {
    gl_Position = ViewProjectionMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}