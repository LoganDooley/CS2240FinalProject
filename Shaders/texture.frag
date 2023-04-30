#version 330 core

out vec4 fragColor;

uniform sampler2D tex;
in vec2 uv;

void main() {
    float value = texture(tex, uv).r;
    if(value > 0){
        fragColor = vec4(value, 0, 0, 1);
    }
    else{
        fragColor = vec4(0, -value, 0, 1);
    }
}
