#version 330 core
#extension GL_ARB_fragment_layer_viewport : enable

const float tau = 6.28318530718f;

in vec2 uv;

const int NUM_K = 4;
uniform int NUM_POS = 4096;
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

vec4 ambientAmplitude(vec2 pos, int i_theta) {
    return (i_theta == 1) ? vec4(0.5) : vec4(0);
}

vec4 lookup_amplitude(int i_x, int i_y, int i_theta) {
    i_theta = (i_theta + NUM_THETA) % NUM_THETA;
    vec2 pos = vec2(i_x + 0.5, i_y + 0.5) / NUM_POS * (maxParam.xy - minParam.xy) + minParam.xy;

    // wavefront unlikely to diverge here, and even if it does they dont diverge by that much
    if (i_x < 0 || i_x >= NUM_POS || i_y < 0 || i_y >= NUM_POS)
        // we need an amplitude for a point outside of the simulation box
        return ambientAmplitude(pos, i_theta);

    return texelFetch(_Amplitude[i_theta], ivec2(i_x, i_y), 0);
}

vec4 evaluate(int itheta) {
    /* int ix = int(uv.x * NUM_POS); */
    /* int iy = int(uv.y * NUM_POS); */

    /* int i_theta = gl_Layer; */
    /* vec2 pos = mix(minParam.xy, maxParam.xy, uv); */

    /* float theta = mix(minParam.z, maxParam.z, i_theta + 0.5); */

    /* vec4 wavenumber = vec4(minParam.w, minParam.w * 2, minParam.w*4, minParam.w*8); */
    /* vec2 wavedirection = vec2(cos(theta), sin(theta)); */

    /* bool atLeast2Away = i_x > 2 && i_x < NUM_POS - 2 && i_y > 2 && i_y < NUM_POS-2; */

    /* vec4 amplitude = texelFetch(_Amplitude, ivec3(uv.x * NUM_POS, uv.y * NUM_POS, i_theta), 0); */

    /* if (atLeast2Away) { */
    /*     vec4 aspeed = advectionSpeed(wavenumber); */

    /*     // found on bottom of page 6 */
    /*     vec4 delta = 1e-5 * unitParam.x * unitParam.x * unitParam.w * unitParam.w * dispersionSpeed(wavenumber); */

    /*     // found on bottom of page 6 */
    /*     vec4 gamma = 0.025 * aspeed * unitParam.z * unitParam.z / unitParam.x; */

    /*     int h = 1; // step size */

    /*     // caching some values common to the calculations below */
    /*     float inverseH2 = 1.0f / float(h*h); */
    /*     float inverse2H = 1.0f / float(2*h); */

    /*     vec4 lookup_xh_y_theta_k = lookup_amplitude(i_x + h, i_y, i_theta); */
    /*     vec4 lookup_xnegh_y_theta_k = lookup_amplitude(i_x - h, i_y, i_theta); */

    /*     vec4 lookup_x_yh_theta_k = lookup_amplitude(i_x, i_y + h, i_theta); */
    /*     vec4 lookup_x_ynegh_theta_k = lookup_amplitude(i_x, i_y - h, i_theta); */


    /*     // use central difference to obtain (k dot V_x) */
    /*     vec4 partialDerivativeWRTX = (lookup_xh_y_theta_k - lookup_xnegh_y_theta_k) * inverse2H; */
    /*     vec4 partialDerivativeWRTY = (lookup_x_yh_theta_k - lookup_x_ynegh_theta_k) * inverse2H; */

    /*     vec4 directionalDerivativeWRTK = vec4( */
    /*         dot(wavedirection, vec2(partialDerivativeWRTX.r, partialDerivativeWRTY.r)), */
    /*         dot(wavedirection, vec2(partialDerivativeWRTX.g, partialDerivativeWRTY.g)), */
    /*         dot(wavedirection, vec2(partialDerivativeWRTX.b, partialDerivativeWRTY.b)), */
    /*         dot(wavedirection, vec2(partialDerivativeWRTX.a, partialDerivativeWRTY.a)) */
    /*     ); */

    /*     // central difference to obtain (k dot V_x)^2 */
    /*     vec4 secondPartialDerivativeWRTX = (lookup_xh_y_theta_k + lookup_xnegh_y_theta_k - 2 * amplitude) * inverseH2; */
    /*     vec4 secondPartialDerivativeWRTY = (lookup_x_yh_theta_k + lookup_x_ynegh_theta_k - 2 * amplitude) * inverseH2; */

    /*     vec4 secondDirectionalDerivativeWRTK = vec4( */
    /*         dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.r, secondPartialDerivativeWRTY.r)), */
    /*         dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.g, secondPartialDerivativeWRTY.g)), */
    /*         dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.b, secondPartialDerivativeWRTY.b)), */
    /*         dot(wavedirection * wavedirection, vec2(secondPartialDerivativeWRTX.a, secondPartialDerivativeWRTY.a)) */
    /*     ); */

    /*     // we are actually using a step size of h/2 here */
    /*     // use central difference to obtain d2A / dtheta^2 numerically */
    /*     vec4 secondPartialDerivativeWRTtheta = (lookup_amplitude(i_x, i_y, i_theta + h) */ 
    /*         + lookup_amplitude(i_x, i_y, i_theta - h) - 2 * amplitude) * inverseH2; */

    /*     // equation 18 */
    /*     vec4 derivativeWRTt = */ 
    /*         -aspeed * directionalDerivativeWRTK + */
    /*         delta * secondDirectionalDerivativeWRTK + */
    /*         gamma * secondPartialDerivativeWRTtheta; */

    /*     /1* amplitude += derivativeWRTt * deltaTime; *1/ */
    /* } */
    return vec4(0);
}

void main() {
    outAmplitude0 = texelFetch(_Amplitude[0], ivec2(uv * NUM_POS), 0);
    outAmplitude1 = texelFetch(_Amplitude[1], ivec2(uv * NUM_POS), 0);
    outAmplitude2 = texelFetch(_Amplitude[2], ivec2(uv * NUM_POS), 0);
    outAmplitude3 = texelFetch(_Amplitude[3], ivec2(uv * NUM_POS), 0);
    outAmplitude4 = texelFetch(_Amplitude[4], ivec2(uv * NUM_POS), 0);
    outAmplitude5 = texelFetch(_Amplitude[5], ivec2(uv * NUM_POS), 0);
    outAmplitude6 = texelFetch(_Amplitude[6], ivec2(uv * NUM_POS), 0);
    outAmplitude7 = texelFetch(_Amplitude[7], ivec2(uv * NUM_POS), 0);
}
