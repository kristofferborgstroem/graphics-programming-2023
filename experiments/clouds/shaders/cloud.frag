#version 330 core

in vec2 TexCoord;
in vec3 VertexCoord;
in vec3 RayDirection;
in vec3 RayOrigin;

out vec4 FragColor;

uniform sampler3D Noise;
uniform sampler3D LowResNoise;
uniform sampler2D WeatherMap;

uniform float CloudAbsorption;
uniform float LightAbsorption;
uniform float WeatherScale;
uniform float Scale;
uniform float Thickness;

uniform float Time;
uniform float AnimationSpeed;

uniform vec3 LightPosition;

uniform int CloudSteps;
uniform int LightSteps;
uniform float BackwardScattering;
uniform float ForwardScattering;

uniform vec3 CloudSampling;

uniform int Debug;

void intersection();

vec3 boundsMin = vec3(-1, 0.9, -1);
vec3 boundsMax = vec3(1, 1, 1);

float map(float n, float inMin, float inMax, float outMin, float outMax) {
    return (n - inMin) * (outMax - outMin) / (inMax - inMin) + outMax;
}

float saturate(float v) {
    return v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
}

float densityAtPoint(vec3 samplePoint) {

    vec2 offSetWeather = 0.5 + WeatherScale * AnimationSpeed * vec2(Time, Time);
    float weather = saturate(texture(WeatherMap, samplePoint.xz * 0.5 * WeatherScale + offSetWeather).r);
    vec3 offSet = Scale * WeatherScale * AnimationSpeed * vec3(Time, 0.0, Time);
    samplePoint = samplePoint * 0.5 * Scale + offSet;
    vec4 noiseSample = texture(Noise, samplePoint);
    vec4 temp = texture(LowResNoise, samplePoint);

    float noiseSampleFBM = (noiseSample.r * CloudSampling.r + noiseSample.g * CloudSampling.g + noiseSample.b * CloudSampling.b) * Thickness;
    //float noiseSampleFBM = map(noiseSample.r, (noiseSample.g * 0.7 + noiseSample.b + 0.3) - 1, 1, 0, 1);
    return saturate(noiseSampleFBM * 2.0 - 1.0) * weather;
    if (noiseSampleFBM > 0.0) {
        noiseSample = texture(Noise, samplePoint);
        float density = (noiseSample.r * CloudSampling.r + noiseSample.g * CloudSampling.g + noiseSample.b * CloudSampling.b) * Thickness;
        return saturate(density * 2.0 - 1.0) * weather;
    }
    return 0.0;
}

vec2 intersection(vec3 point, vec3 direction)
{
    vec3 inv = 1.0 / direction;

    float tx1 = (boundsMin.x - point.x) * inv.x;
    float tx2 = (boundsMax.x - point.x) * inv.x;

    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);

    float ty1 = (boundsMin.y - point.y) * inv.y;
    float ty2 = (boundsMax.y - point.y) * inv.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    float tz1 = (boundsMin.z - point.z) * inv.z;
    float tz2 = (boundsMax.z - point.z) * inv.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    return vec2(tmin, tmax);
}

float henyey(float angle, float g) {
    return ((1 - g * g) / pow((1 + g * g - 2 * g * angle) * 4 * 3.14, 1.5));
}

float scattering(float angle) {
    float hg = henyey(angle, ForwardScattering) * 0.5 + henyey(angle, -BackwardScattering) * 0.5;
    float threshold = 0.5;
    return threshold + hg * (1 - threshold);
}

float distanceToLightBorder(vec3 samplePoint) {
    vec3 pointToLight = normalize(LightPosition - samplePoint);
    vec2 lightrayIntersection = intersection(samplePoint, pointToLight);
    return lightrayIntersection.y;
}

float densityTowardsLight(vec3 samplePoint) {

    // Far is distance to border. We won't need near when intersecting inside box.
    vec3 pointToLight = normalize(LightPosition - samplePoint);
    vec2 lightrayIntersection = intersection(samplePoint, pointToLight);
    float distToBorder = lightrayIntersection.y;

    int steps = LightSteps;
    float stepSize = distToBorder / steps;
    vec3 dirScaled = pointToLight * stepSize;

    samplePoint += dirScaled;
    //vec3 dirScaled = pointToLight * stepSize;
    float acc = 0.0;
    float traveled = stepSize;
    while (traveled < distToBorder) {
        float density = densityAtPoint(samplePoint);
        acc += density * stepSize;
        if (acc > 0.95) break;
        samplePoint += dirScaled;
        traveled += stepSize;
    }

    float darkness = 0.4;

    return darkness + exp(-acc * LightAbsorption) * (1 - darkness);
}

vec4 rayMarch() {
    vec3 rayDir = normalize(RayDirection);
    vec2 rayIntersection = intersection(RayOrigin, rayDir);

    if (rayIntersection.x >= rayIntersection.y) return vec4(0.0);

    float near = max(rayIntersection.x, 0);
    float far = rayIntersection.y;
    float distance = far - near;

    vec3 samplePoint = RayOrigin + near * rayDir;
    int steps = CloudSteps;
    float stepSize = distance / steps;

    vec3 RayDirectionScaled = rayDir * stepSize;

    float threshold = 0.95;
    float energy = 0.0;
    float transmittance = 1.0;
    float traveled = stepSize;

    float hg = scattering(dot(normalize(LightPosition), rayDir));

    for (int i = 0; i < steps; i++) {
        float density = densityAtPoint(samplePoint);

        if (density > 0.0) {
            float t = densityTowardsLight(samplePoint);
            energy += density * stepSize * (1 - t) * transmittance * (1 - hg);
            transmittance *= exp(-traveled * density * CloudAbsorption);
        }

        if (transmittance < 0.1) break;
        samplePoint += RayDirectionScaled;
        traveled += stepSize;
    }

    return vec4(vec3(1 - energy * 20), 1 - transmittance);
}

void main () {

    if (Debug == 0) {
        FragColor = rayMarch();
    } else if (Debug == 1) {
        FragColor = vec4(vec3(texture(Noise, vec3(TexCoord, 0.0)).r), 1.0);
    } else if (Debug == 2) {
        FragColor = vec4(vec3(texture(Noise, vec3(TexCoord, 0.0)).g), 1.0);
    } else {
        FragColor = vec4(vec3(texture(Noise, vec3(TexCoord, 0.0)).b), 1.0);
    }
}