#version 330 core

//layout (location = 0) out float value;

// k = 0.001 to k = 100 is the approximate valid range
// k = 0.001->0.01 (0.005), 0.01->0.1 (0.05), 0.1->1 (0.5), 1->100 (50)

//out vec4 fragColor;

out vec4 profileBuffer;

uniform float t;
uniform int pResolution = 512;
uniform float windSpeed;
uniform float D;
uniform float kMax;
uniform float kMin;

const int integration_nodes = 100;
const float tau = 6.28318531;
const float pi = 3.1415926;

float rand(float k){
    return fract(sin(k * 125612.9898) * 43758.5453);
}

float w(float k){
    return sqrt(5 * k * 9.81);
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

float psiBarIntegrand(float k, float p, float dk){
    float waveLength = tau / k;
    float w = w(k);

    return JONSWAP(w) * cos(k * p - w * t) * k;
}

float psiBar(float p){
    float dk = (kMax - kMin)/integration_nodes;
    float k = kMin;
    float result = 0;
    for(int i = 0; i<integration_nodes; i++){
        result += psiBarIntegrand(k, p, dk) * dk;
        k += dk;
    }
    return result;
}

float computeProfileBuffer(float p){
    float s = p / D;
    float h00 = 2 * pow(s, 3) - 3 * pow(s, 2) + 1;
    float h01 = -2 * pow(s, 3) + 3 * pow(s, 2);
    return 0.5 * (psiBar(p) + h00 * psiBar(p + D) + h01 * psiBar(p - D));
}

void main() {
    float p = D * floor(gl_FragCoord.x) / pResolution;

    profileBuffer = 100 * vec4(computeProfileBuffer(p), 0, 0, 0);
}
