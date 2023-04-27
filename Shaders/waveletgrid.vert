#version 330 core

layout (location = 0) in vec3 pos;

out VS_OUT {
    vec2 uv;
} vs_out;

vec2 viewportToUV(vec3 viewport) {
    return vec2((viewport + 1) / 2);
}

void main() {
    gl_Position = vec4(pos,1);
    uv = viewportToUV();
}
