#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 view, projection;

out vec3 worldSpace_pos;

void main() {
    vec2 xz = vec2(pos.x, pos.z);
    vec4 worldSpace_pos4 = projection * view * vec4(worldSpace_pos, 1);
    worldSpace_pos = vec3(worldSpace_pos4.x, worldSpace_pos4.y, worldSpace_pos4.z);

    gl_Position = worldSpace_pos4;
}
