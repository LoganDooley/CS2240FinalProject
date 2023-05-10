#version 330 core

out vec4 fragColor;

uniform sampler2D heightMap;
uniform sampler2D boundaryMap;
uniform sampler2D gradientMap;
uniform int whichVisualization;
in vec2 uv;

void main() {
    if (whichVisualization == 0) {
        vec4 value = texture(heightMap, uv);
        /* fragColor = vec4(vec3(value.r), 1); */
        // both 
        fragColor = vec4(vec3(value.gb, value.r > 0.641 ? 1 : 0), 1);
    } else if (whichVisualization == 1) {
        vec4 value = texture(boundaryMap, uv);
        fragColor = vec4(vec3(value.r > 0), 1);
    } else if (whichVisualization == 2) {
        vec4 value = texture(gradientMap, uv);
        fragColor = vec4(vec3((value.rg + 1) / 2, 0), 1);
    } else if (whichVisualization == 3) {
        vec4 value = texture(heightMap, uv);
        fragColor = vec4(vec3(value.r), 1);
    }
    /* fragColor = vec4(vec3(abs(value.r)), 1); */
    /* fragColor = vec4(vec3(value.r <= 0.641 ? 1 : 0), 1); */
    /* fragColor = vec4(vec3(value.r > 0.641 ? 1 : 0), 1); */
    /* fragColor = vec4(vec3((value.rg + 1) / 2, 0), 1); */
}
