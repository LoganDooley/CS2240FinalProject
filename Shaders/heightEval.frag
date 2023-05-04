#version 330 core

out float fragColor;

const int NUM_THETA_SIMULATED = 8;
uniform sampler2D _Amplitudes[8];

uniform sampler2D profileBuffers;
uniform int pb_resolution = 4096;
uniform vec2 gridSpacing;
uniform vec2 bottomLeft;
uniform int thetaResolution;
uniform int kResolution;
uniform float windTheta = 0;

float getPeriod(int ik){
    if(ik == 0){
        return 2 * 3.14159/0.0001;
    }
    if(ik == 1){
        return 2 * 3.14159/0.001;
    }
    if(ik == 2){
        return 2 * 3.14159/0.01;
    }
    if(ik == 3){
        return 2 * 3.14159/1;
    }
}

float rand(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

float pbValue(float p, int ik){
    int N = pb_resolution;
    float ip = N * p / getPeriod(ik);
    int pLower = int(floor(ip));

    vec4 color = texture(profileBuffers, vec2(ip/pb_resolution, 0.5));
    if(ik == 0){
        return color.r;
    }
    if(ik == 1){
        return color.g;
    }
    if(ik == 2){
        return color.b;
    }
    if(ik == 3){
        return color.a;
    }
}

float PositiveCosineSquaredDS(float theta){
    if(theta > 3.14159){
        theta -= 2 * 3.14159;
    }
    float angle = theta - windTheta;
    if(angle > -3.14159/2 && angle < 3.14159/2){
        return (2/3.14159) * pow(cos(angle), 2);
    }
    else return 0;
}

vec4 amplitude(vec2 uv, float thetaUV) {
    float thetaTexCoord = (thetaUV * NUM_THETA_SIMULATED) - 0.5;
    if (thetaTexCoord < 0) thetaTexCoord += NUM_THETA_SIMULATED;

    int itheta_simulated = int(thetaTexCoord);
    int itheta_simulated_p1 = (itheta_simulated + 1) % NUM_THETA_SIMULATED;

    vec4 v0 = texture(_Amplitudes[itheta_simulated], uv);
    vec4 v1 = texture(_Amplitudes[itheta_simulated_p1], uv);

    float t = thetaTexCoord - itheta_simulated;

    return mix(v0, v1, vec4(t));
}

void main() {
    vec2 pos = bottomLeft + vec2((gl_FragCoord.x - 0.5), (gl_FragCoord.y - 0.5)) * gridSpacing;
    vec2 uv = gl_FragCoord.xy / pb_resolution;

    float height = 0;
    int DIR_NUM = thetaResolution;
    float da = 6.28318530718 / DIR_NUM;
    for(int itheta = 0; itheta < DIR_NUM; itheta++){

        float angle = itheta * da;

        vec4 amp = amplitude(uv, float(itheta + 0.5) / DIR_NUM);

        for(int ik = 0; ik < kResolution; ik++){
            vec2 kdir = vec2(cos(angle), sin(angle));
            float p = dot(kdir, pos) + rand( kdir );

            /* height += da * sqrt(da * 0.01 * PositiveCosineSquaredDS(angle)) * pbValue(p, ik); */
            height += da * amp[ik] * pbValue(p, ik);
            //height += da * 0.01 * pbValue(p, ik);

//            int interpolated_ik = int(round(4 * float(ik) / kResolution));
//            interpolated_ik = clamp(interpolated_ik, 0, 3);

//            height += da * amp[interpolated_ik] * pbValue(p, ik);
        }
    }

    fragColor = height;
}
