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
    return sqrt(k * 9.81);
}

float JONSWAP(float w){
    float U = windSpeed;
    float F = 1000;
    float g = 9.8;
    float a = 0.076 * pow(U*U/(F*g), 0.22);
    float wp = 22 * (g*g/(U*F));
    float gamma = 3.3;
    float sigma = 0.07;
    if(w > wp){
        sigma = 0.09;
    }
    float r = exp(-pow(w-wp, 2)/(2 * pow(sigma, 2) * pow(wp, 2)));
    return ((a * pow(g, 2))/pow(w, 5)) * exp(-(5.f/4) * pow(wp/w, 4)) * pow(gamma, 4);
}

float PiersonMoskowitz(float w){
    float a = 8.1 * pow(10, -3);
    float U = windSpeed * 10;
    float b = 0.74;
    float g = 9.8;
    float w0 = g/(1.026 * U);
    return ((a * pow(g, 2))/pow(w, 5)) * exp(-b * pow(w0/w, 4));
}

float Kitaigorodskii(float w, float h){
    float g = 9.8;
    float wh = w * sqrt(h/g);
    if(wh <= 1){
        return 0.5 * pow(wh, 2);
    }
    else{
        return 1 - 0.5 * pow(2 - wh, 2);
    }
}

float TMA(float w, float h){
    return JONSWAP(w) * Kitaigorodskii(w, h);
}

float psiBarInteggand(float k, float p, float dk){
    float waveLength = 2 * 3.1415 /k;
    float w = w(k);
    return sqrt(2 * JONSWAP(w) * dk) * waveLength * cos(k * p - w * t);
}

float psiBar(float p, int integration_nodes, float k_min, float k_max){
    float dk = k_min;
    float k = k_min;

    float result = 0;
    for(int i = 0; i<integration_nodes; i++){
        result += psiBarIntegrand(k, p, dk) * dk;
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
