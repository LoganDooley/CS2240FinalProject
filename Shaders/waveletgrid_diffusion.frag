#version 330 core

in vec2 uv;

const int NUM_K = 4;
uniform int NUM_POS = 2048;
uniform int NUM_THETA = 8;

uniform vec4 wavenumberValues;
uniform vec4 angularFrequency;
uniform vec4 advectionSpeed;
uniform vec4 dispersionSpeed;

uniform sampler2D _Amplitude[8];
uniform sampler2D _Height;
uniform sampler2D _Gradient;
uniform sampler2D _CloseToBoundary;
uniform float waterLevel = 0.641;

uniform vec2 waveDirections[8];
uniform vec4 ambient[8];
uniform vec2 windDirection;

uniform float time;
uniform float deltaTime;

uniform vec4 minParam;
uniform vec4 maxParam;
uniform vec4 unitParam;

layout (location = 0) out vec4 outAmplitude0;
layout (location = 1) out vec4 outAmplitude1;
layout (location = 2) out vec4 outAmplitude2;
layout (location = 3) out vec4 outAmplitude3;
layout (location = 4) out vec4 outAmplitude4;
layout (location = 5) out vec4 outAmplitude5;
layout (location = 6) out vec4 outAmplitude6;
layout (location = 7) out vec4 outAmplitude7;

vec4 lookup_amplitude(int ix, int iy, int itheta) {
    itheta = (itheta + NUM_THETA) % NUM_THETA;
    ix = clamp(ix, 0, NUM_POS - 1);
    iy = clamp(iy, 0, NUM_POS - 1);
    /* if (ix < 0 || ix >= NUM_POS || iy < 0 || iy >= NUM_POS) { */
    /*     // clamp, so there's not a sharp discontinuety */
    /*     ix = clamp(); */
    /*     return ambient[itheta]; */
    /* } */
    return texelFetch(_Amplitude[itheta], ivec2(ix, iy), 0);
}

vec4 evaluate(int ix, int iy, int itheta, vec4 amplitude) {
    vec4 wavenumber = wavenumberValues;
    vec2 wavedirection = waveDirections[itheta];

    // found on bottom of page 6
    float wavenumberResolution = (wavenumber.w - wavenumber.x) / 4;
    vec4 delta = 1e-2 * unitParam.x * unitParam.x * wavenumberResolution * wavenumberResolution * 
        abs(dispersionSpeed);

    // found on bottom of page 6
    vec4 gamma = 0.025 * advectionSpeed * unitParam.z * unitParam.z / unitParam.x;

    // caching some values common to the calculations below
    vec4 lookup_xh_y_theta_k = lookup_amplitude(ix + 1, iy, itheta);
    vec4 lookup_xnegh_y_theta_k = lookup_amplitude(ix - 1, iy, itheta);

    vec4 lookup_x_yh_theta_k = lookup_amplitude(ix, iy + 1, itheta);
    vec4 lookup_x_ynegh_theta_k = lookup_amplitude(ix, iy - 1, itheta);

    // use central difference to obtain (k dot V_x)
    vec4 partialDerivativeWRTX = (lookup_xh_y_theta_k - lookup_xnegh_y_theta_k) / 2;
    vec4 partialDerivativeWRTY = (lookup_x_yh_theta_k - lookup_x_ynegh_theta_k) / 2;

    vec4 directionalDerivativeWRTK = partialDerivativeWRTX * wavedirection.x + partialDerivativeWRTY * wavedirection.y;

    // central difference to obtain (k dot V_x)^2
    vec4 secondPartialDerivativeWRTX = (lookup_xh_y_theta_k + lookup_xnegh_y_theta_k - 2 * amplitude);
    vec4 secondPartialDerivativeWRTY = (lookup_x_yh_theta_k + lookup_x_ynegh_theta_k - 2 * amplitude);

    vec4 secondDirectionalDerivativeWRTK = 
        secondPartialDerivativeWRTX * wavedirection.x * wavedirection.x + secondPartialDerivativeWRTY * wavedirection.y * wavedirection.y;

    // we are actually using a step size of h/2 here
    // use central difference to obtain d2A / dtheta^2 numerically
    vec4 secondPartialDerivativeWRTtheta = (lookup_amplitude(ix, iy, itheta + 1) 
        + lookup_amplitude(ix, iy, itheta - 1) - 2 * amplitude);

    // equation 18
    vec4 derivativeWRTt = vec4(0);

    derivativeWRTt += -advectionSpeed * directionalDerivativeWRTK; // first term. resists the change in Amplitude
    derivativeWRTt += delta * secondDirectionalDerivativeWRTK; // second term, dampen in k
    derivativeWRTt += gamma * secondPartialDerivativeWRTtheta; // third term, angular diffusion.

    return max(amplitude + derivativeWRTt * deltaTime, vec4(0));
}

void main() {
    int ix = int(uv.x * NUM_POS);
    int iy = int(uv.y * NUM_POS);
    vec2 pos = mix(minParam.xy, maxParam.xy, uv);

    /* bool atLeast2Away = ix >= 2 && iy < NUM_POS - 2 && iy > 2 && iy < NUM_POS-2; */
    bool atLeast2Away = texelFetch(_CloseToBoundary, ivec2(ix, iy), 0).r == 0;
    /* bool atLeast2Away = true; */

    outAmplitude0 = texelFetch(_Amplitude[0], ivec2(uv * NUM_POS), 0);
    outAmplitude1 = texelFetch(_Amplitude[1], ivec2(uv * NUM_POS), 0);
    outAmplitude2 = texelFetch(_Amplitude[2], ivec2(uv * NUM_POS), 0);
    outAmplitude3 = texelFetch(_Amplitude[3], ivec2(uv * NUM_POS), 0);
    outAmplitude4 = texelFetch(_Amplitude[4], ivec2(uv * NUM_POS), 0);
    outAmplitude5 = texelFetch(_Amplitude[5], ivec2(uv * NUM_POS), 0);
    outAmplitude6 = texelFetch(_Amplitude[6], ivec2(uv * NUM_POS), 0);
    outAmplitude7 = texelFetch(_Amplitude[7], ivec2(uv * NUM_POS), 0);

    if (atLeast2Away) {
        outAmplitude0 = evaluate(ix, iy, 0, outAmplitude0);
        outAmplitude1 = evaluate(ix, iy, 1, outAmplitude1);
        outAmplitude2 = evaluate(ix, iy, 2, outAmplitude2);
        outAmplitude3 = evaluate(ix, iy, 3, outAmplitude3);
        outAmplitude4 = evaluate(ix, iy, 4, outAmplitude4);
        outAmplitude5 = evaluate(ix, iy, 5, outAmplitude5);
        outAmplitude6 = evaluate(ix, iy, 6, outAmplitude6);
        outAmplitude7 = evaluate(ix, iy, 7, outAmplitude7);
    }
}
