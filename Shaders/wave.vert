#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 view, projection;
uniform sampler2D heightMap;
uniform vec2 lowerLeft;
uniform vec2 upperRight;

out vec3 worldSpace_pos;
out vec3 surfaceNormal;
out vec3 incidentDirection;
out float useShader;

out float height;

void main() {
    useShader = 1;
    if (pos.x < lowerLeft.x || pos.z < lowerLeft.y || pos.x > upperRight.x || pos.z > upperRight.y) {
        worldSpace_pos = vec3(pos.x, 0, pos.z);
    } else {
        worldSpace_pos = vec3(pos.x, texture(heightMap, (vec2(pos.x, pos.z) - lowerLeft)/(upperRight - lowerLeft)).r, pos.z);
    }
    if (useShader > 0) {
        vec3 dfdx = vec3(1, 0, texture(heightMap, (vec2(pos.x + 1, pos.z) - lowerLeft)/(upperRight - lowerLeft)).r - texture(heightMap, (vec2(pos.x - 1, pos.z) - lowerLeft)/(upperRight - lowerLeft)).r);
        vec3 dfdy = vec3(0, 1, texture(heightMap, (vec2(pos.x, pos.z + 1) - lowerLeft)/(upperRight - lowerLeft)).r - texture(heightMap, (vec2(pos.x, pos.z - 1) - lowerLeft)/(upperRight - lowerLeft)).r);
        surfaceNormal = normalize(cross(dfdx, dfdy));
        // Directional Light Example
        incidentDirection = normalize(vec3(0.5, -1, 0.5));
        // Point Light Example
        //incidentDirection = normalize(vec3(0, 1, 0) - worldSpace_pos);
    }
    height = texture(heightMap, (vec2(pos.x, pos.z) - lowerLeft)/(upperRight - lowerLeft)).r;
    gl_Position = projection * view * vec4(worldSpace_pos, 1);
}
