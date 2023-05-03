#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 view, projection;
uniform sampler2D heightMap;
uniform vec2 lowerLeft;
uniform vec2 upperRight;

out vec3 worldSpace_pos;

out float height;

void main() {
    if(pos.x < lowerLeft.x || pos.z < lowerLeft.y || pos.x > upperRight.x || pos.z > upperRight.y){
        worldSpace_pos = vec3(pos.x, 0, pos.z);
    }
    else{
        worldSpace_pos = vec3(pos.x, texture(heightMap, (vec2(pos.x, pos.z) - lowerLeft)/(upperRight - lowerLeft)).r, pos.z);
    }

    height = texture(heightMap, (vec2(pos.x, pos.z) - lowerLeft)/(upperRight - lowerLeft)).r;
    gl_Position = projection * view * vec4(worldSpace_pos, 1);
}
