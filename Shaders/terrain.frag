#version 330 core

out vec4 fragColor;

in vec3 worldSpace_pos;
in vec3 worldSpace_normal;

void main() {
    fragColor = vec4(vec3(worldSpace_pos.y), 1);
}
