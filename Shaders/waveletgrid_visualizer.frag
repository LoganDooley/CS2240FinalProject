#version 330 core

in vec2 uv;

uniform int NUM_POS = 4096;
uniform sampler3D _Amplitude;
uniform int thetaIndex;

out vec4 color;

void main() {
    vec4 amplitudeSampled = texelFetch(_Amplitude, ivec3(uv.x * NUM_POS, uv.y * NUM_POS, thetaIndex), 0);
    color = amplitudeSampled;
}
