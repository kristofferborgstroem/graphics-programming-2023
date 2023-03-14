#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in vec4 LightSpacePosition;

out vec4 FragColor;

// Albedo
uniform vec4 Color;
uniform sampler2D Albedo;
uniform sampler2D DepthMap;
uniform float DrawShadow;

// Light
uniform vec3 LightColor;
uniform vec3 LightPosition;

// Ambient
uniform vec3 AmbientColor;
uniform float AmbientReflection;

// Diffuse
uniform float DiffuseReflection;

// Specular
uniform float SpecularReflection;
uniform float SpecularExponent;

// Camera
uniform vec3 CameraPosition;

float ShadowCalculation() {
    vec3 lightCoord = LightSpacePosition.xyz / LightSpacePosition.w;
    vec3 sampleCoord = lightCoord.xyz * 0.5 + 0.5;
    float depth = sampleCoord.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(DepthMap, 0);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            float pcfDepth = texture(DepthMap, sampleCoord.xy + vec2(x, y) * texelSize).r;
            shadow += depth > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9;

    texture(DepthMap, sampleCoord.xy).r;
    if (depth > 1.0) {
        return 0.0;
    } else {
        return shadow;
    }
}

vec3 GetAmbientReflection() {
    return AmbientColor * AmbientReflection * Color.xyz;
}

vec3 GetDiffuseReflection(vec3 L, vec3 N) {
    return LightColor * DiffuseReflection * Color.xyz * max(dot(N, L), 0.0);
}

vec3 GetSpecularReflection(vec3 N, vec3 H) {
    return LightColor * SpecularReflection * pow(max(dot(N, H), 0.0), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec3 L, vec3 N, vec3 H) {
    return GetAmbientReflection() + (1.0 - ShadowCalculation()) * (GetDiffuseReflection(L, N) + GetSpecularReflection(N, H));
}


void main()
{
    vec3 L = normalize(LightPosition - WorldPosition);
    vec3 N = normalize(WorldNormal);
    vec3 V = normalize(CameraPosition - WorldPosition);
    vec3 H = normalize(L + V);
    FragColor = vec4(GetBlinnPhongReflection(L, N, H), 1.0) * texture(Albedo, TexCoord);
}
