#version 330 core

layout (location = 0) in vec3 pos;

struct Wave{
    float wavelength;
    float steepness;
    float dispersion;
    float wavenumber;
    float amplitude;
    vec2 direction;
};

uniform Wave waves[16];

uniform int numWaves = 0;

uniform float t = 0;

uniform mat4 view, projection;

out vec3 worldSpace_pos;
out vec3 worldSpace_norm;

vec3 gerstnerWavePos(vec2 xz, float t){
    float x = xz.x;
    float z = xz.y;
    float y = 0;

    for(int i = 0; i<numWaves; i++){
        float Ai = waves[i].amplitude;
        vec2 Di = waves[i].direction;
        float wi = waves[i].wavenumber;
        float phii = waves[i].dispersion;
        float Qi = waves[i].steepness / (wi * Ai * numWaves);

        float arg = wi * dot(Di, xz) + phii * t;
        float cosarg = cos(arg);
        float sinarg = sin(arg);

        x += Qi * Ai * Di.x * cosarg;
        z += Qi * Ai * Di.y * cosarg;
        y += Ai * sinarg;
    }

    return vec3(x, y, z);
}

vec3 gerstnerWaveNorm(vec2 xz, float t){
    float x = 0;
    float z = 0;
    float y = 1;

    for(int i = 0; i<numWaves; i++){
        float Ai = waves[i].amplitude;
        vec2 Di = waves[i].direction;
        float wi = waves[i].wavenumber;
        float phii = waves[i].dispersion;
        float Qi = waves[i].steepness / (wi * Ai * numWaves);

        float WA = wi * Ai;
        float arg = wi * dot(Di, xz) + phii * t;
        float cosarg = cos(arg);
        float sinarg = sin(arg);

        x -= Di.x * WA * cosarg;
        z -= Di.y * WA * cosarg;
        y -= Qi * WA * sinarg;
    }

    return vec3(x, y, z);
}

void main() {
    vec2 xz = vec2(pos.x, pos.z);
    worldSpace_pos = gerstnerWavePos(xz, t);
    worldSpace_norm = gerstnerWaveNorm(xz, t);

    gl_Position = projection * view * vec4(worldSpace_pos, 1);
}
