#version 330 core

in vec2 uv;

uniform int NUM_POS = 4096;
uniform sampler2D _Amplitude;
uniform int thetaIndex;

out vec4 color;

void main() {
    vec4 amplitudeSampled = texelFetch(_Amplitude, ivec2(uv.x * NUM_POS, uv.y * NUM_POS), 0);
    color = vec4(vec3(amplitudeSampled.rgb), 1);
}
