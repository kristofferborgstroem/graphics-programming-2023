#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D AlbedoWater;
uniform vec2 TextureScale;
uniform vec2 TextureOffset;

void main()
{
    FragColor = vec4(texture(AlbedoWater, (TexCoord + TextureOffset) * TextureScale).rgb * (WorldPosition.y + 2.0), 1.0) * Color;
}
