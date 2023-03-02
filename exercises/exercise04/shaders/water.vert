#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform float CurrentTime;
uniform float WaveLength;
uniform float WaveAmplitude;

void main()
{
    float temp = CurrentTime;
    vec3 world = (WorldMatrix * vec4(VertexPosition, 1.0)).xyz;

    float Height = sin(-world.x * WaveLength - CurrentTime) * WaveAmplitude;

    WorldPosition = vec3(world.x, world.y + Height, world.z);
    WorldNormal = (WorldMatrix * vec4(VertexNormal, 0.0)).xyz;
    TexCoord = VertexTexCoord;
    gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
