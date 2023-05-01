#version 330 core

out float fragColor;

uniform sampler2D profileBuffers;
uniform int pb_resolution = 4096;
uniform float periods[8];
uniform vec2 gridSpacing;
uniform vec2 bottomLeft;
uniform int thetaResolution;
uniform int kResolution;

float pbValue(float p, int ik){
    int N = pb_resolution;
    float ip = N * p / periods[ik];
    int pLower = int(floor(ip));
    return texture(profileBuffers, vec2(ip/pb_resolution, ik)).r;



    float wpUpper = ip - pLower;
    pLower = int(mod(pLower, N));
    int pUpper = int(mod(pLower + 1, N));
    return wpUpper * texture(profileBuffers, vec2((pUpper + 0.5)/pb_resolution, (ik + 0.5)/kResolution)).r + (1 - wpUpper) * texture(profileBuffers, vec2((pLower + 0.5)/pb_resolution, (ik + 0.5)/kResolution)).r;
}

void main() {
    vec2 pos = bottomLeft + vec2((gl_FragCoord.x - 0.5), (gl_FragCoord.y - 0.5)) * gridSpacing;

    float height = 0;
    int DIR_NUM = thetaResolution;
    for(int ik = 0; ik < kResolution; ik++){
        float da = 6.28318530718 / DIR_NUM;
        for(int itheta = 0; itheta < DIR_NUM; itheta++){
            float angle = itheta * da;
            vec2 kdir = vec2(cos(angle), sin(angle));
            float kdir_x = dot(kdir, pos);
            height += da * max(dot(kdir, normalize(pos)), 0) * pbValue(kdir_x, ik);
        }
    }

    fragColor = height;
}
