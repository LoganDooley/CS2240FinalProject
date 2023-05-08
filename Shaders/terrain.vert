#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4 view, projection, model;

out vec3 worldSpace_pos;
out vec3 worldSpace_normal;

void main() {
    worldSpace_pos = vec3(model * vec4(pos, 1));
    worldSpace_normal = vec3(transpose(inverse(model))*vec4(normal, 0.0));

    gl_Position = projection * view * vec4(worldSpace_pos, 1);
}
