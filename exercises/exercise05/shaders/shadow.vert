#version 330 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 Model;
uniform mat4 LightSpaceMatrix;

void main() {
    gl_Position = LightSpaceMatrix * Model * vec4(VertexPosition, 1.0);
}