#version 330 core

//layout (location = 0) out float value;

out float fragColor;

uniform float t;
uniform int pResolution = 4096;
uniform int integration_nodes;
uniform float windSpeed;
uniform float kMin;

float rand(float k){
    return fract(sin(k * 125612.9898) * 43758.5453);
}

float w(float k){
    return sqrt(k/50 * 9.81);
}

float phillips(float w){
    return 1/(5 * w);
    return 8.1 * pow(10, -3) * 2 * 3.1415 * pow(9.8, 2)/pow(w, 5);
}

float psiBarIntegrand(float k, float p){
    float waveLength = 1/k;
    float w = w(k);
    return phillips(w) * waveLength * cos(k * p - w * t + 2 * 3.1415 * rand(k));
}

float psiBar(float p, int integration_nodes, float k_min, float k_max){
    float dk = k_min;
    float k = k_min;

    float result = 0;
    for(int i = 0; i<integration_nodes; i++){
        result += psiBarIntegrand(k, p) * dk;
        k += dk;
    }

    return result;
}

void main() {
    int ik = int(floor(gl_FragCoord.y)); // Get row of texture corresponding to z index
    int ip = int(floor(gl_FragCoord.x)); // Get column of texture corresponding to p index

    float k_min = kMin * pow(integration_nodes + 1, ik);
    float k_max = kMin * pow(integration_nodes + 1, ik + 1);

    float period = 6.28318530718 / k_min;
    float p = (ip * period) / pResolution;

    fragColor = psiBar(p, integration_nodes, k_min, k_max);
}
