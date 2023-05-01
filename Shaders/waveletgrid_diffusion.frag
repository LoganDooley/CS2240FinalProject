#version 330 core
#extension GL_ARB_fragment_layer_viewport : enable

in vec2 uv;

const int NUM_K = 4;
uniform int NUM_POS = 4096;
uniform int NUM_THETA = 8;

const float gravity = 9.81;
const float surfaceTension = 72.8 / 1000; // of water

uniform sampler3D _Amplitude;
uniform sampler2D _AtLeast2Away;

uniform float deltaTime;

uniform vec4 minParam;
uniform vec4 maxParam;
uniform vec4 unitParam;

out vec4 outAmplitude;

vec4 angularFrequency(vec4 wavenumber) {
    return pow(wavenumber * gravity +
            surfaceTension * wavenumber * wavenumber * wavenumber, vec4(0.5));
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

vec4 ambientAmplitude(vec2 pos) {
    return vec4(0);
}

vec4 lookup_amplitude(int i_x, int i_y, int i_theta) {
    i_theta = (i_theta + NUM_THETA) % NUM_THETA;
    vec2 pos = (unitParam.xy / 2 + vec2(i_x, i_y) / NUM_POS) * (maxParam.xy - minParam.xy) + minParam.xy;

    // wavefront unlikely to diverge here, and even if it does they dont diverge by that much
    if (i_x < 0 || i_x >= NUM_POS || i_y < 0 || i_y >= NUM_POS)
        // we need an amplitude for a point outside of the simulation box
        return ambientAmplitude(pos);

    return texelFetch(_Amplitude, ivec3(i_x, i_y, i_theta), 0);
}

void main() {
    int i_x = int(uv.x * NUM_POS);
    int i_y = int(uv.y * NUM_POS);
    vec2 pos = mix(minParam.xy, maxParam.xy, vec2(uv));

    int i_theta = gl_Layer;
    float theta = float(i_theta) / NUM_THETA + unitParam.z / 2;

    vec4 wavenumber = vec4(minParam.w, minParam.w * 2, minParam.w*4, minParam.w*8);
    vec2 wavedirection = vec2(cos(theta), sin(theta));

    bool atLeast2Away = true;

    vec4 amplitude = texelFetch(_Amplitude, ivec3(uv.x * NUM_POS, uv.y * NUM_POS, i_theta), 0);

    if (atLeast2Away) {
        vec4 aspeed = advectionSpeed(wavenumber);

        // found on bottom of page 6
        vec4 delta = 1e-5 * unitParam.x * unitParam.x * unitParam.w * unitParam.w * dispersionSpeed(wavenumber);

        // found on bottom of page 6
        vec4 gamma = 0.025 * aspeed * unitParam.z * unitParam.z / unitParam.x;

        int h = 1; // step size

        // caching some values common to the calculations below
        float inverseH2 = 1.0f / (h*h);
        float inverse2H = 1.0f / (2*h);

        vec4 lookup_xh_y_theta_k = lookup_amplitude(i_x + h, i_y, i_theta);
        vec4 lookup_xnegh_y_theta_k = lookup_amplitude(i_x - h, i_y, i_theta);

        vec4 lookup_x_yh_theta_k = lookup_amplitude(i_x, i_y + h, i_theta);
        vec4 lookup_x_ynegh_theta_k = lookup_amplitude(i_x, i_y - h, i_theta);

        // we are actually using a step size of h/2 here
        // use central difference to obtain d2A / dtheta^2 numerically
        vec4 secondPartialDerivativeWRTtheta = (lookup_amplitude(i_x, i_y, i_theta + h) 
            + lookup_amplitude(i_x, i_y, i_theta - h) - 2 * amplitude) * inverseH2;

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

        // equation 18
        vec4 derivativeWRTt = -aspeed * directionalDerivativeWRTK + delta * secondDirectionalDerivativeWRTK + gamma * secondPartialDerivativeWRTtheta;

        /* amplitude += derivativeWRTt * deltaTime; */
    }

    outAmplitude = amplitude;
}
