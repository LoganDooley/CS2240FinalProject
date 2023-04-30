#version 330 core

out vec4 fragColor;

uniform sampler2D profileBuffers;
uniform int pb_resolution = 4096;
uniform float periods[8];
uniform vec2 gridSpacing;
uniform vec2 bottomLeft;
uniform int thetaResolution;
uniform int zResolution;

float pbValue(float p, int iz){
    int N = pb_resolution;
    float ip = N * p / periods[iz];
    int pLower = int(floor(ip));
    float wpUpper = ip - pLower;
    pLower = int(mod(pLower, N));
    int pUpper = int(mod(pLower + 1, N));
    return wpUpper * texture(profileBuffers, vec2(pUpper + 0.5, iz + 0.5)).r + (1 - wpUpper) * texture(profileBuffers, vec2(pLower + 0.5, iz + 0.5)).r;
}

void main() {
    vec2 pos = bottomLeft + vec2((gl_FragCoord.x - 0.5), (gl_FragCoord.y - 0.5)) * gridSpacing;

    float height = 0;
    int DIR_NUM = thetaResolution;
    for(int iz = 0; iz < zResolution; iz++){
        float da = 6.28318530718 / DIR_NUM;
        for(int itheta = 0; itheta < DIR_NUM; itheta++){
            float angle = itheta * da * 6.28318530718;
            vec2 kdir = vec2(cos(angle), sin(angle));
            float kdir_x = dot(kdir, pos);
            if(itheta == 0 || itheta == 3){
                height += da * 0.01 * pbValue(kdir_x, iz);
            }
        }
    }

    float result = (3.14159/2 + atan(height))/3.14159;
    fragColor = vec4(vec3(result), 1);
    //fragColor = vec4(pos, 1, 1);
    //fragColor = vec4(vec3(pbValue(1, 1)), 1);
}
