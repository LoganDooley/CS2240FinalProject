#version 330 core

uniform sampler2D heightMap;

in vec3 worldSpace_pos;
in vec2 uv;

out vec4 fragColor;

void main() {
    float value = texture(heightMap, uv).r;
    if(value > 0){
        fragColor = vec4(value, 0, 0, 1);
    }
    else{
        fragColor = vec4(0, -value, 0, 1);
    }
    //fragColor = vec4(vec3(0.5 + 0.5 * value), 1);
    //fragColor = vec4(vec3(color), 1);
    //fragColor = vec4(texture(heightMap, uv).r, 0, 0, 1);
}
