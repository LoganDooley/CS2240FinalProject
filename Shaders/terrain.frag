#version 330 core

out vec4 fragColor;

uniform float height0 = 5;
uniform float height1 = 50;
uniform float height2 = 120;
uniform vec3 c0 = vec3(63, 29, 44) / 255.0;
uniform vec3 c1 = vec3(140, 114, 164) / 255.0;
uniform vec3 c2 = vec3(231, 213, 226) / 255.0;

in vec3 worldSpace_pos;
in vec3 worldSpace_normal;

void main() {
    float height = worldSpace_pos.y;
    if (height <= 0) fragColor = vec4(c0, 1);
    else if (height <= height1) {
        float t = (height - height0) / (height1-height0);
        fragColor = vec4(mix(c0, c1, t), 1);
    }
    else if (height <= height2) {
        float t = (height - height1) / (height2-height1);
        fragColor = vec4(mix(c1, c2, t*t), 1);
    }
    else fragColor = vec4(c2, 1);
}
