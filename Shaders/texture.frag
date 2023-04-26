#version 330 core

out vec4 fragColor;

uniform sampler2D tex;
in vec2 uv;

void main() {
    fragColor = vec4(vec3(texture(tex, uv.r)), 1);
}
