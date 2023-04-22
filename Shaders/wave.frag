#version 330 core

in vec3 worldSpace_pos;

out vec4 fragColor;

void main() {
    float value = abs(worldSpace_pos.y);
    value = max(min(value, 1), 0);
    fragColor = vec4(vec3(value), 1);
}
