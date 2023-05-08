#version 330 core

out vec4 fragColor;

uniform sampler2D tex;
in vec2 uv;

void main() {
    /* float value = texture(tex, uv).r; */
    vec3 value = texture(tex, uv).xyz;
    /* fragColor = vec4(vec3(value <= 0.641 ? 1 : 0), 1); */
    /* fragColor = vec4(uv, 0, 1); */
    fragColor = vec4(abs(value), 1);
}
