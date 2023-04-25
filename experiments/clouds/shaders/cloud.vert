#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 TextureCoord;

out vec2 TexCoord;
out vec3 VertexCoord;
out vec3 RayDirection;
out vec3 RayOrigin;


uniform mat4 WorldMatrix;
uniform mat4 ViewProjectionMatrix;
uniform vec4 LocalCameraPosition;

void main () {
    TexCoord = TextureCoord;
    VertexCoord = VertexPosition;
    RayDirection = VertexPosition - LocalCameraPosition.xyz;
    RayOrigin = LocalCameraPosition.xyz;
    gl_Position = ViewProjectionMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}
