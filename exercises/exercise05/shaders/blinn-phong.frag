#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

// Albedo
uniform vec4 Color;
uniform sampler2D Albedo;

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
    return GetAmbientReflection() + GetDiffuseReflection(L, N) + GetSpecularReflection(N, H);
}

void main()
{
    vec3 L = normalize(LightPosition - WorldPosition);
    vec3 N = normalize(WorldNormal);
    vec3 V = normalize(CameraPosition - WorldPosition);
    vec3 H = normalize(L + V);
    FragColor = texture(Albedo, TexCoord) * vec4(GetBlinnPhongReflection(L, N, H), 1.0);
}
