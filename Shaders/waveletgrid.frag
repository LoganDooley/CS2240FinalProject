#version 330 core

in vec2 uv;

const int NUM_K = 4;

const float gravity = 9.81;
const float surfaceTension = 72.8 / 1000; // of water

uniform sampler3D _Amplitude;
uniform float deltaTime;

uniform int resolutionTheta = 8;
uniform int zetaIndex;

uniform vec4 minParam;
uniform vec4 maxParam;
uniform vec4 unitParam;

out vec4 amplitude;

// TODO: Precompute these for grid points
float angularFrequency(float wavenumber) {
    return sqrt(wavenumber * gravity +
            surfaceTension * wavenumber * wavenumber * wavenumber);
}

// TODO: Precompute these for grid points
float WaveletGrid::advectionSpeed(float wavenumber) {
    float numerator = (gravity + 3 * surfaceTension * wavenumber * wavenumber);
    float denominator = 2 * angularFrequency(wavenumber);
    return numerator / denominator;
}

// TODO: Precompute these for grid points
float WaveletGrid::dispersionSpeed(float wavenumber) {
    // courtesy of wolfram alpha
    // https://www.wolframalpha.com/input?i=d%5E2%2Fdx%5E2%28sqrt%28ax%2Bbx%5E3%29%29
    float numerator =
        (-2 * gravity + 6 * gravity * surfaceTension * wavenumber * wavenumber +
         3 * surfaceTension * surfaceTension * wavenumber * wavenumber * wavenumber * wavenumber);
    float denom = 4 * pow(wavenumber * (gravity + surfaceTension * wavenumber * wavenumber), 3 / 2);
    return numerator / denom;
}

float ambientAmplitude(vec4 pos) {
    return 0;
}

void main() {
    int thetaIndex = gl_Layer;

    // ADVECTION STEP
    float result[NUM_K];
#pragma openNV (unroll all)
    for (int zetaIndex = 0; zetaIndex < NUM_K; zetaIndex++) {
        vec4 pos = mix(minParam, maxParam, 
            vec4(uv, float(thetaIndex) / resolutionTheta, float(zetaIndex) / NUM_K)) + unitParam/2;
        // since we're using zeta
        pos.w = pow(2, zetaIndex) * minParam.w;

        float theta = pos.z;
        float wavenumber = pos.w;
        vec2 wavedirection = vec2(cos(theta), sin(theta));

        float omega = advectionSpeed(wavenumber);

        vec4 nxtPos = pos - vec4(deltaTime * wavedirection * omega, 0, 0);

        vec3 nxtPosUV = ((nxtPos - unitParam/2) - minParam) / (maxParam - minParam);

        // this uses texture interpolation, and not the thing recoomended in the paper.
        // TODO: write an actual interpolator.
        float interpolatedAmplitude = texture(_Amplitude, nxtPosUV)[zetaIndex];

        // ambient amplitude if outside grid
        if (pos.x < minParam.x || pos.x >= maxParam.x || 
            pos.x < minParam.x || pos.x >= maxParam.x) 
                interpolatedAmplitude = ambientAmplitude(pos);

        amplitude[zetaIndex] = interpolatedAmplitude;
    }
}
