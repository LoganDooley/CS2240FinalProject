#version 330 core

in vec2 uv;

const int NUM_K = 4;

const float gravity = 9.81;
const float surfaceTension = 72.8 / 1000; // of water

uniform sampler3D _Amplitude;
uniform float deltaTime;

uniform int resolutionTheta = 8;
uniform int resolutionPos = 4096;

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

float interpolate(vec4 v, float t) {
    float dk = (v[2] - v[0]) / 2;
    float dkp1 = (v[3] - v[1]) / 2;
    float deltaK = v[2] - v[1];

    float signDK = sign(dk);
    float signDKp1 = sign(dkp1);
    float signdeltaK = sign(deltaK);

    // if delta is 0 or the signed bit of dk, dkp1, and deltaK differs
    // this is used to force monotonicity of f(t) on the interval [tk, tk+1]
    // so that the interpolation is more stable and is less prone to overshooting
    if (deltaK == 0 || (signDK + signDKp1 + signdeltaK) != 3 * signdeltaK)
        deltaK = dk = dkp1 = 0;

    // table-based approach of evaluating a 3rd order polynomial
    vec4 a = vec4( v[1], dk, 3 * deltaK - 2 * dk - dkp1, dk + dkp1 - deltaK );
    vec4 b = vec4(1, t, t*t, t*t*t);

    return dot(a,b);
}

vec4 interpolate(float x, float y, int iTheta, int iZeta) {
    int ix = x;
    int iy = y;

    vec4 w;

#pragma openNV (unroll all)
    for (int dx = 0; dx < 4; dx++) {
        vec4 v = vec4(
            texelFetch(ivec3(ix-1, iy, iTheta))[iZeta],
            texelFetch(ivec3(ix, iy, iTheta))[iZeta],
            texelFetch(ivec3(ix+1, iy, iTheta))[iZeta],
            texelFetch(ivec3(ix+2, iy, iTheta))[iZeta]
        );

        w[dx] = interpolate(v, y - iy);
    }

    return interpolate(w, x-ix);
}

void main() {
    int thetaIndex = gl_Layer;

    vec4 interpolatedAmplitudes[NUM_K];

    // ADVECTION STEP
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

        vec2 nxtPosUV = vec2(((nxtPos - unitParam/2) - minParam) / (maxParam - minParam));

        // this uses texture interpolation, and not the thing recoomended in the paper.
        vec4 interpolatedAmplitude = 
            interpolate(nxtPosUV.x * resolutionPos, nxtPosUV.y * resolutionPos, thetaIndex, zetaIndex);

        // ambient amplitude if outside grid
        if (pos.x < minParam.x || pos.x >= maxParam.x || 
            pos.x < minParam.x || pos.x >= maxParam.x) 
                interpolatedAmplitude = ambientAmplitude(pos).rrrr;

        interpolatedAmplitudes[zetaIndex] = interpolatedAmplitude;
    }

    amplitude = vec4(
        interpolatedAmplitudes[0].r,
        interpolatedAmplitudes[1].b,
        interpolatedAmplitudes[2].g,
        interpolatedAmplitudes[3].a
    );
}
