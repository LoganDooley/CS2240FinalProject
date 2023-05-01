#version 330 core
#extension GL_ARB_fragment_layer_viewport : enable

in vec2 uv;

// DIMENSIONS
const int NUM_K = 4; // this must not be higher than 4
uniform int NUM_THETA = 8;
uniform int NUM_POS = 4096;

uniform float gravity = 9.81;
uniform float surfaceTension = 72.8 / 1000; // of water

uniform sampler3D _Amplitude;

uniform float deltaTime;
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

float ambientAmplitude(vec4 pos) {
    return 1;
    /* return (gl_Layer == 1) ? 1 : 0; */
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

float interpolate2D(float x, float y, int iTheta, int iZeta) {
    int ix = int(x);
    int iy = int(y);

    vec4 w;

#pragma openNV (unroll all)
    for (int dx = 0; dx < 4; dx++) {
        vec4 v = vec4(
            texelFetch(_Amplitude, ivec3(ix-1, iy, iTheta), 0)[iZeta],
            texelFetch(_Amplitude, ivec3(ix, iy, iTheta), 0)[iZeta],
            texelFetch(_Amplitude, ivec3(ix+1, iy, iTheta), 0)[iZeta],
            texelFetch(_Amplitude, ivec3(ix+2, iy, iTheta), 0)[iZeta]
        );

        w[dx] = interpolate(v, y - iy);
    }

    return interpolate(w, x-ix);
}

void main() {
    int thetaIndex = gl_Layer;

    // ADVECTION STEP
#pragma openNV (unroll all)
    for (int zetaIndex = 0; zetaIndex < NUM_K; zetaIndex++) {
        vec4 pos = mix(minParam, maxParam,
            vec4(uv, float(thetaIndex) / NUM_THETA, float(zetaIndex) / NUM_K)) + unitParam/2;
        // since we're using zeta
        pos.w = pow(2, zetaIndex) * minParam.w;

        float theta = pos.z;
        float wavenumber = pos.w;
        vec2 wavedirection = vec2(cos(theta), sin(theta));

        float omega = advectionSpeed(wavenumber);

        vec2 nxtPos = pos.xy - deltaTime * wavedirection * omega;

        vec2 nxtPosUV = ((nxtPos - unitParam.xy/2) - minParam.xy) / (maxParam.xy - minParam.xy);

        // this uses texture interpolation, and not the thing recoomended in the paper.
        float interpolatedAmplitude = 
            interpolate2D(nxtPosUV.x * NUM_POS, nxtPosUV.y * NUM_POS, thetaIndex, zetaIndex);
        
        // ambient amplitude if outside grid
        if (nxtPos.x < minParam.x || nxtPos.x >= maxParam.x || 
            nxtPos.y < minParam.y || nxtPos.y >= maxParam.y) 
                interpolatedAmplitude = ambientAmplitude(pos);

        amplitude[zetaIndex] = interpolatedAmplitude;
    }
}
