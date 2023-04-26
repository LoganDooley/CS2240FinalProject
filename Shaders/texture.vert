#version 330 core

layout (location = 0) in vec3 pos;

out vec2 uv;

void main() {
    uv = vec2((pos.x + 1)/2, (pos.y + 1)/2);
    gl_Position = vec4(pos, 1);
}
