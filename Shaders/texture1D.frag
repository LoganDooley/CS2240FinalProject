#version 330 core

out vec4 fragColor;

uniform sampler1D tex;
in vec2 uv;

void main() {
    vec4 color = texture(tex, uv.x);
    float value = color.r;
    value = (atan(value) + 3.14159/2)/3.14159;
    fragColor = vec4(vec3(value), 1);
}
