#version 330 core

in vec3 worldSpace_pos;

in float height;

out vec4 fragColor;

void main() {
    float value = worldSpace_pos.y;
    if(value > 0){
        fragColor = vec4(value, 0, 0, 1);
    }
    else{
        fragColor = vec4(0, -value, 0, 1);
    }
    fragColor = vec4(vec3(0.5 + 0.5 * value) * vec3(0, 0, 1), 1);

    fragColor.r = height;
}
