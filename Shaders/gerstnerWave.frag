#version 330 core

in vec3 worldSpace_pos;
in vec3 worldSpace_norm;

out vec4 fragColor;

void main() {
    vec3 norm = normalize(worldSpace_norm);
    float vert = dot(norm, vec3(0, 1, 0));
    vec3 blue = vec3(0, 0.4118, 0.5804);
    fragColor = vec4(blue * vert, 1);
}