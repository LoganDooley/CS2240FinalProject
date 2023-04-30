#version 330 core

layout (location = 0) in vec3 pos;

out vec2 uv;

vec2 viewportToUV(vec3 viewport) {
    return vec2((viewport + 1) / 2);
}

void main() {
    gl_Position = vec4(pos,1);
    uv = viewportToUV(pos);
}
