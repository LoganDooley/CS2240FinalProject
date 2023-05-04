#version 330 core
#extension GL_ARB_fragment_layer_viewport : enable

const float tau = 6.28318530718f;

in vec2 uv;

const int NUM_K = 4;
uniform int NUM_POS = 2048;
uniform int NUM_THETA = 8;

const float gravity = 9.81;
const float surfaceTension = 72.8 / 1000; // of water

uniform vec4 wavenumberValues;

uniform sampler2D _Amplitude[8];
uniform sampler2D _AtLeast2Away;

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

vec4 angularFrequency(vec4 wavenumber) {
    return pow(wavenumber * gravity + surfaceTension * wavenumber * wavenumber * wavenumber, vec4(0.5));
}

vec4 advectionSpeed(vec4 wavenumber) {
    vec4 numerator = (gravity + 3 * surfaceTension * wavenumber * wavenumber);
    vec4 denominator = 2 * angularFrequency(wavenumber);
    return numerator / denominator;
}

vec4 dispersionSpeed(vec4 wavenumber) {
    // courtesy of wolfram alpha
    // https://www.wolframalpha.com/input?i=d%5E2%2Fdx%5E2%28sqrt%28ax%2Bbx%5E3%29%29
    vec4 numerator =
        (-2 * gravity + 6 * gravity * surfaceTension * wavenumber * wavenumber +
         3 * surfaceTension * surfaceTension * wavenumber * wavenumber * wavenumber * wavenumber);
    vec4 denom = 4 * pow(wavenumber * (gravity + surfaceTension * wavenumber * wavenumber), vec4(3 / 2.0));
    return numerator / denom;
}

vec4 lookup_amplitude(int ix, int iy, int itheta) {
    itheta = (itheta + NUM_THETA) % NUM_THETA;
    /* vec2 pos = vec2(ix + 0.5, iy + 0.5) / NUM_POS * (maxParam.xy - minParam.xy) + minParam.xy; */

    // wavefront unlikely to diverge here, and even if it does they dont diverge by that much
    if (ix < 0 || ix >= NUM_POS || iy < 0 || iy >= NUM_POS)
        // we need an amplitude for a point outside of the simulation box
        return (itheta == 0) ? vec4(1) : vec4(0);

    return texelFetch(_Amplitude[itheta], ivec2(ix, iy), 0);
}

vec4 evaluate(int ix, int iy, int itheta, vec4 amplitude) {
    float theta = mix(minParam.z, maxParam.z, (itheta + 0.5) / NUM_THETA);

    vec4 wavenumber = wavenumberValues;
    vec2 wavedirection = vec2(cos(theta), sin(theta));

    vec4 aspeed = advectionSpeed(wavenumber);

    // found on bottom of page 6
    float wavenumberResolution = (wavenumber.w - wavenumber.x) / 4;
    vec4 delta = 1e-5 * unitParam.x * unitParam.x * wavenumberResolution * wavenumberResolution * abs(dispersionSpeed(wavenumber));

    // found on bottom of page 6
    vec4 gamma = 0.025 * aspeed * unitParam.z * unitParam.z / unitParam.x;

    int h = 1; // step size

    // caching some values common to the calculations below
    float inverseH2 = 1.0f / float(h*h);
    float inverse2H = 1.0f / float(2*h);

    vec4 lookup_xh_y_theta_k = lookup_amplitude(ix + h, iy, itheta);
    vec4 lookup_xnegh_y_theta_k = lookup_amplitude(ix - h, iy, itheta);

    vec4 lookup_x_yh_theta_k = lookup_amplitude(ix, iy + h, itheta);
    vec4 lookup_x_ynegh_theta_k = lookup_amplitude(ix, iy - h, itheta);


    // use central difference to obtain (k dot V_x)
    vec4 partialDerivativeWRTX = (lookup_xh_y_theta_k - lookup_xnegh_y_theta_k) * inverse2H;
    vec4 partialDerivativeWRTY = (lookup_x_yh_theta_k - lookup_x_ynegh_theta_k) * inverse2H;

    vec4 directionalDerivativeWRTK = vec4(
        dot(wavedirection, vec2(partialDerivativeWRTX.r, partialDerivativeWRTY.r)),
        dot(wavedirection, vec2(partialDerivativeWRTX.g, partialDerivativeWRTY.g)),
        dot(wavedirection, vec2(partialDerivativeWRTX.b, partialDerivativeWRTY.b)),
        dot(wavedirection, vec2(partialDerivativeWRTX.a, partialDerivativeWRTY.a))
    );

    // central difference to obtain (k dot V_x)^2
    vec4 secondPartialDerivativeWRTX = (lookup_xh_y_theta_k + lookup_xnegh_y_theta_k - 2 * amplitude) * inverseH2;
    vec4 secondPartialDerivativeWRTY = (lookup_x_yh_theta_k + lookup_x_ynegh_theta_k - 2 * amplitude) * inverseH2;

    vec4 secondDirectionalDerivativeWRTK = vec4(
        dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.r, secondPartialDerivativeWRTY.r)),
        dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.g, secondPartialDerivativeWRTY.g)),
        dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.b, secondPartialDerivativeWRTY.b)),
        dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.a, secondPartialDerivativeWRTY.a))
    );

    // we are actually using a step size of h/2 here
    // use central difference to obtain d2A / dtheta^2 numerically
    vec4 secondPartialDerivativeWRTtheta = (lookup_amplitude(ix, iy, itheta + h) 
        + lookup_amplitude(ix, iy, itheta - h) - 2 * amplitude) * inverseH2;

    // equation 18
    vec4 derivativeWRTt = vec4(0);

    derivativeWRTt += -aspeed * directionalDerivativeWRTK; // first term. resists the change in Amplitude
    derivativeWRTt += delta * secondDirectionalDerivativeWRTK; // second term, dampen in k
    derivativeWRTt += gamma * secondPartialDerivativeWRTtheta; // third term, angular diffusion.


    return derivativeWRTt * deltaTime;
}

void main() {
    int ix = int(uv.x * NUM_POS);
    int iy = int(uv.y * NUM_POS);
    vec2 pos = mix(minParam.xy, maxParam.xy, uv);

    bool atLeast2Away = ix >= 2 && iy < NUM_POS - 2 && iy > 2 && iy < NUM_POS-2;

    outAmplitude0 = texelFetch(_Amplitude[0], ivec2(uv * NUM_POS), 0);
    outAmplitude1 = texelFetch(_Amplitude[1], ivec2(uv * NUM_POS), 0);
    outAmplitude2 = texelFetch(_Amplitude[2], ivec2(uv * NUM_POS), 0);
    outAmplitude3 = texelFetch(_Amplitude[3], ivec2(uv * NUM_POS), 0);
    outAmplitude4 = texelFetch(_Amplitude[4], ivec2(uv * NUM_POS), 0);
    outAmplitude5 = texelFetch(_Amplitude[5], ivec2(uv * NUM_POS), 0);
    outAmplitude6 = texelFetch(_Amplitude[6], ivec2(uv * NUM_POS), 0);
    outAmplitude7 = texelFetch(_Amplitude[7], ivec2(uv * NUM_POS), 0);

    if (atLeast2Away) {
        outAmplitude0 += evaluate(ix, iy, 0, outAmplitude0);
        outAmplitude1 += evaluate(ix, iy, 1, outAmplitude1);
        outAmplitude2 += evaluate(ix, iy, 2, outAmplitude2);
        outAmplitude3 += evaluate(ix, iy, 3, outAmplitude3);
        outAmplitude4 += evaluate(ix, iy, 4, outAmplitude4);
        outAmplitude5 += evaluate(ix, iy, 5, outAmplitude5);
        outAmplitude6 += evaluate(ix, iy, 6, outAmplitude6);
        outAmplitude7 += evaluate(ix, iy, 7, outAmplitude7);
    }

}
