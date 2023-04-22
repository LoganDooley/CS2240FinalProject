#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 view, projection;

out vec3 worldSpace_pos;

void main() {
    vec2 xz = vec2(pos.x, pos.z);

    worldSpace_pos = pos;

    gl_Position = projection * view * vec4(pos, 1);
}
