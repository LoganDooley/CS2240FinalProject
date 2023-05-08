#version 330 core

out vec4 fragColor;

uniform sampler2D tex;
in vec2 uv;

void main() {
    vec4 value = texture(tex, uv);
    fragColor = vec4(vec3(abs(value.r)), 1);
}
