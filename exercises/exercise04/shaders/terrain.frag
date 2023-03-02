#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D AlbedoDirt;
uniform vec2 DirtRange;
uniform sampler2D AlbedoGrass;
uniform vec2 GrassRange;
uniform sampler2D AlbedoRock;
uniform vec2 RockRange;
uniform sampler2D AlbedoSnow;
uniform vec2 TextureScale;

// Map x to range [0.0; 1.0]
float normalize(float x, vec2 range) {
    return (x - range.x) / (range.y - range.x);
}

float map(vec2 range, float height) {
    float t = normalize(mix(range.x, range.y, normalize(height, range)), range);
    return clamp (t, 0.0, 1.0);
}

// range : [0.2; 0.4]
// in    : 0.3
// out   : 0.5

void main()
{
    float Height = WorldPosition.y;
    vec4 dirt = texture(AlbedoDirt, TexCoord * TextureScale);
    vec4 grass = texture(AlbedoGrass, TexCoord * TextureScale);
    vec4 rock = texture(AlbedoRock, TexCoord * TextureScale);
    vec4 snow = texture(AlbedoSnow, TexCoord * TextureScale);
    vec4 fst = mix(dirt, grass, map(DirtRange, Height));
    vec4 snd = mix(fst, rock, map(GrassRange, Height));
    FragColor = mix(snd, snow, map(RockRange, Height)) * Color;
}
