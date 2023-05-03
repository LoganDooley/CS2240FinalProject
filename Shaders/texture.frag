#version 330 core

out vec4 fragColor;

uniform sampler2D tex;
in vec2 uv;

void main() {
    vec3 value = texture(tex, uv).xyz;
    fragColor = vec4(abs(value), 1);
}
