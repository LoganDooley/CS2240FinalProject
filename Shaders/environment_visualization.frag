#version 330 core

out vec4 fragColor;

const float tau = 6.28318530718f;

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
        fragColor = vec4(vec3(value.gb, 0), 1);
        /* fragColor = vec4(vec3(value.gb, value.r > 0.641 ? 1 : 0), 1); */
    } else if (whichVisualization == 1) {
        vec4 value = texture(boundaryMap, uv);
        fragColor = vec4(vec3(value.r > 0), 1);
    } else if (whichVisualization == 2) {
        /* vec2 value = normalize(texture(gradientMap, uv).rg); */
        /* fragColor = vec4(vec3((value.rg + 1) / 2, 0), 1); */
        float value = texture(gradientMap, uv).r;
        float inDomain = texture(heightMap, uv).r - 0.641;
        if (inDomain <= 0)
            fragColor = vec4(0,0,0,1);
        else fragColor = vec4(vec3(value), 1);
        /* else if (value < 0) */
        /*     fragColor = vec4(0.5, 0.5, 0.5, 1); */
        /* else if (value <= 1.00/3) */
        /*     fragColor = vec4(mix(vec3(1, 0, 0), vec3(0, 1, 0), value / 3), 1); */
        /* else if (value <= 2.00/3) */
        /*     fragColor = vec4(mix(vec3(0, 1, 0), vec3(0, 0, 1), value / 3 - 1), 1); */
        /* else if (value <= 1) */
        /*     fragColor = vec4(mix(vec3(0, 0, 1), vec3(1, 0, 0), value / 3 - 2), 1); */
        /* else */ 
        /*     fragColor = vec4(1, 1, 1, 1); */
    } else if (whichVisualization == 3) {
        float value = texture(gradientMap, uv).r;
        float inDomain = texture(heightMap, uv).r - 0.641;
        if (inDomain <= 0)
            fragColor = vec4(0,0,0,1);
        else if (value < 0)
            fragColor = vec4(0.5, 0.5, 0.5, 1);
        else if (value <= 1.00/3)
            fragColor = vec4(mix(vec3(1, 0, 0), vec3(0, 1, 0), value * 3.00f), 1);
        else if (value <= 2.00/3)
            fragColor = vec4(mix(vec3(0, 1, 0), vec3(0, 0, 1), value * 3.00f - 1), 1);
        else if (value <= 1)
            fragColor = vec4(mix(vec3(0, 0, 1), vec3(1, 0, 0), value * 3.00f - 2), 1);
        else 
            fragColor = vec4(1, 1, 1, 1);
    } else if (whichVisualization == 4) {
        vec4 value = texture(heightMap, uv);
        fragColor = vec4(vec3(value.r), 1);
    }
    /* fragColor = vec4(vec3(abs(value.r)), 1); */
    /* fragColor = vec4(vec3(value.r <= 0.641 ? 1 : 0), 1); */
    /* fragColor = vec4(vec3(value.r > 0.641 ? 1 : 0), 1); */
    /* fragColor = vec4(vec3((value.rg + 1) / 2, 0), 1); */
}
