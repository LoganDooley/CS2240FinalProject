#version 330 core
#extension GL_ARB_fragment_layer_viewport : enable

in vec2 uv;

const float tau = 6.28318530718f;

// DIMENSIONS
const int NUM_K = 4; // this must not be higher than 4
uniform int NUM_THETA = 8;
uniform int NUM_POS = 2048;

uniform float gravity = 9.81;
uniform float surfaceTension = 72.8 / 1000; // of water

uniform vec4 wavenumberValues;

uniform sampler2D _Amplitude[8];

uniform float time;
uniform float deltaTime;
uniform vec4 minParam;
uniform vec4 maxParam;
uniform vec4 unitParam;

layout (location = 0) out vec4 amplitude0;
layout (location = 1) out vec4 amplitude1;
layout (location = 2) out vec4 amplitude2;
layout (location = 3) out vec4 amplitude3;
layout (location = 4) out vec4 amplitude4;
layout (location = 5) out vec4 amplitude5;
layout (location = 6) out vec4 amplitude6;
layout (location = 7) out vec4 amplitude7;

// TODO: Precompute these for grid points
float angularFrequency(float wavenumber) {
    return sqrt(wavenumber * gravity + surfaceTension * wavenumber * wavenumber * wavenumber);
}

// TODO: Precompute these for grid points
float advectionSpeed(float wavenumber) {
    float numerator = (gravity + 3 * surfaceTension * wavenumber * wavenumber);
    float denominator = 2 * angularFrequency(wavenumber);
    return numerator / denominator;
}

// TODO: Precompute these for grid points
float dispersionSpeed(float wavenumber) {
    // courtesy of wolfram alpha
    // https://www.wolframalpha.com/input?i=d%5E2%2Fdx%5E2%28sqrt%28ax%2Bbx%5E3%29%29
    float numerator =
        (-2 * gravity + 6 * gravity * surfaceTension * wavenumber * wavenumber +
         3 * surfaceTension * surfaceTension * wavenumber * wavenumber * wavenumber * wavenumber);
    float denom = 4 * pow(wavenumber * (gravity + surfaceTension * wavenumber * wavenumber), 3 / 2);
    return numerator / denom;
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
    if (deltaK == 0 || signDK != signdeltaK || signDKp1 != signdeltaK) {
        dk = dkp1 = 0;
    }

    // table-based approach of evaluating a 3rd order polynomial
    vec4 a = vec4( v[1], dk, 3 * deltaK - 2 * dk - dkp1, dk + dkp1 - 2 * deltaK );
    vec4 b = vec4(1, t, t*t, t*t*t);

    return dot(a,b);
}

float get(int ix, int iy, int itheta, int izeta) {
    float ambient = itheta == 0 ? 0.5 : 0.0;
    return iy >= 0 && iy < NUM_POS ? texelFetch(_Amplitude[itheta], ivec2(ix, iy), 0)[izeta] : ambient;
}

// we can break this into 2 shaders and instead of doing 16 texture fetches, we only
// do 8 but i dont know if that's that much better
float interpolate2D(float x, float y, int itheta, int izeta) {
    int ix = int(floor(x));
    int iy = int(floor(y));

    vec4 w;

#pragma openNV (unroll all)
    for (int dy = 0; dy < 4; dy++) {
        int niy = iy+dy-1;
        vec4 v = vec4(
            get(ix-1,   niy, itheta, izeta),
            get(ix,     niy, itheta, izeta),
            get(ix+1,   niy, itheta, izeta),
            get(ix+2,   niy, itheta, izeta)
        );
        float ambient = (itheta == 0 ? 1 : 0.0);

        w[dy] = niy >= 0 && niy < NUM_POS  ? interpolate(v, x - ix) : ambient;
    }

    return interpolate(w, y-iy);
}

vec4 evaluate(int thetaIndex) {
    vec3 uvw = vec3(uv, (thetaIndex + 0.5) / NUM_THETA);
    vec3 pos = mix(minParam.xyz, maxParam.xyz, uvw);
    float theta = pos.z;

    vec2 wavedirection = vec2(cos(theta), sin(theta));

    vec4 amplitude;

#pragma openNV (unroll all)
    for (int zetaIndex = 0; zetaIndex < NUM_K; zetaIndex++) {
        // since we're using zeta
        float wavenumber = wavenumberValues[zetaIndex];

        vec2 nxtPos = pos.xy - deltaTime * wavedirection * advectionSpeed(wavenumber);

        vec2 nxtPosUV = (nxtPos - minParam.xy) / (maxParam.xy - minParam.xy);
        vec2 nxtPosTexCoord = nxtPosUV * NUM_POS - 0.5;

        float interpolatedAmplitude =
            /* texture(_Amplitude[thetaIndex], nxtPosUV)[zetaIndex]; */
            interpolate2D(nxtPosTexCoord.x, nxtPosTexCoord.y, thetaIndex, zetaIndex);

        // ambient amplitude if outside grid
        if (nxtPos.x < minParam.x || nxtPos.x >= maxParam.x || 
            nxtPos.y < minParam.y || nxtPos.y >= maxParam.y) {

            interpolatedAmplitude = thetaIndex==0 ? 1 : 0;
        }

        amplitude[zetaIndex] = interpolatedAmplitude;
    }

    return amplitude;
}

void main() {
    amplitude0 = evaluate(0);
    amplitude1 = evaluate(1);
    amplitude2 = evaluate(2);
    amplitude3 = evaluate(3);
    amplitude4 = evaluate(4);
    amplitude5 = evaluate(5);
    amplitude6 = evaluate(6);
    amplitude7 = evaluate(7);
}
