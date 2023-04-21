#version 330 core

in vec3 worldSpace_pos;

out vec4 fragColor;

void main() {
    vec3 norm = normalize(worldSpace_pos);
    vec3 blue = vec3(0, 0.4118, 0.5804);
    fragColor = vec4(blue * abs(norm.y), 1);
}
