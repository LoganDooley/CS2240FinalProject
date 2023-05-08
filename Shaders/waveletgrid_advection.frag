#version 330 core

in vec2 uv;

const float tau = 6.28318530718f;
const float pi = 3.141592653589793f;

// DIMENSIONS
const int NUM_K = 4; // this must not be higher than 4
uniform int NUM_THETA = 8;
uniform int NUM_POS = 2048;

uniform float gravity = 9.81;
uniform float surfaceTension = 72.8 / 1000; // of water

uniform vec4 wavenumberValues;

uniform vec2 windDirection;

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

// Temporary randomness for raindrops
float rand(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

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

float ambient(int itheta) {
    // TEMPORARY: FOR RAIN
    //return 0;
    //

    // all of this can be precomputed on the cpu
    float theta = mix(minParam.z, maxParam.z, (itheta + 0.5) / NUM_THETA);
    theta = 2 * 3.14159 * itheta/NUM_THETA;

    vec2 wavedirection = vec2(cos(theta), sin(theta));
    float windSpeed = length(windDirection);

    float cosTheta = dot(wavedirection, windDirection) / windSpeed;

    //return cosTheta < 0 ? 0 : cosTheta;

    return cosTheta < 0 ? 0 : cosTheta * cosTheta * 2 / pi;
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

// courtesy of
// https://jbrd.github.io/2020/12/27/monotone-cubic-interpolation.html
float guaranteeMonotonicInterpolate(vec4 v, float t) {
    float s_minus_1 = v[1] - v[0];
    float s_0 = v[2] - v[1];
    float s_1 = v[3] - v[2];

    // Use central differences to calculate initial gradients at the end-points
    float m_0 = (s_minus_1 + s_0) * 0.5;
    float m_1 = (s_0 + s_1) * 0.5;

    // If the central curve (joining y0 and y1) is neither increasing or decreasing, we
    // should have a horizontal line, so immediately set gradients to zero here.
    if (abs(v[1] - v[2]) < 0.0001) {
        m_0 = 0.0;
        m_1 = 0.0;
    } else {
        // If the curve to the left is horizontal, or the sign of the secants on either side
        // of the end-point are different, set the gradient to zero...
        if (abs(v[0] - v[1]) < 0.0001 || s_minus_1 < 0.0 && s_0 >= 0.0 || s_minus_1 > 0.0 && s_0 <= 0.0)
            m_0 = 0.0;
        // ... otherwise, ensure the magnitude of the gradient is constrained to 3 times the
        // left secant, and 3 times the right secant (whatever is smaller)
        else m_0 *= min(min(3.0 * s_minus_1 / m_0, 3.0 * s_0 / m_0), 1.0);

        // If the curve to the right is horizontal, or the sign of the secants on either side
        // of the end-point are different, set the gradient to zero...
        if (abs(v[2] - v[3]) < 0.0001 || s_0 < 0.0 && s_1 >= 0.0 || s_0 > 0.0 && s_1 <= 0.0)
            m_1 = 0.0;
        // ... otherwise, ensure the magnitude of the gradient is constrained to 3 times the
        // left secant, and 3 times the right secant (whatever is smaller)
        else 
            m_1 *= min(min(3.0 * s_0 / m_1, 3.0 * s_1 / m_1), 1.0);
    }

    // Evaluate the cubic hermite spline
    float result = (((((m_0 + m_1 - 2.0 * s_0) * t) + (3.0 * s_0 - 2.0 * m_0 - m_1)) * t) + m_0) * t + v[1];

    // The values at the end points (y0 and y1) define an interval that the curve passes
    // through. Since the curve between the end-points is now monotonic, all interpolated
    // values between these end points should be inside this interval. However, floating
    // point rounding error can still lead to values slightly outside this range.
    // Guard against this by clamping the interpolated result to this interval...
    return clamp(result, min(v[1], v[2]), max(v[1], v[2]));
}

float get(int ix, int iy, int itheta, int izeta) {
    float ambientAmplitude = ambient(itheta);
    return iy >= 0 && iy < NUM_POS ? texelFetch(_Amplitude[itheta], ivec2(ix, iy), 0)[izeta] : ambientAmplitude;
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
        float ambientAmplitude = ambient(itheta);
        w[dy] = niy >= 0 && niy < NUM_POS  ? guaranteeMonotonicInterpolate(v, x - ix) : ambientAmplitude;
    }

    return guaranteeMonotonicInterpolate(w, y-iy);
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

            interpolatedAmplitude = ambient(thetaIndex);
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

//    // TEMPORARY FOR RAIN: REMOVE LATER
//    if(rand(uv * time) > 0.999){
//        amplitude0 += vec4(0, 0, 0, 0.5);
//        amplitude1 += vec4(0, 0, 0, 0.5);
//        amplitude2 += vec4(0, 0, 0, 0.5);
//        amplitude3 += vec4(0, 0, 0, 0.5);
//        amplitude4 += vec4(0, 0, 0, 0.5);
//        amplitude5 += vec4(0, 0, 0, 0.5);
//        amplitude6 += vec4(0, 0, 0, 0.5);
//        amplitude7 += vec4(0, 0, 0, 0.5);
//    }
    //
    /* amplitude0 = vec4(ambient(0)); */
    /* amplitude1 = vec4(ambient(1)); */
    /* amplitude2 = vec4(ambient(2)); */
    /* amplitude3 = vec4(ambient(3)); */
    /* amplitude4 = vec4(ambient(4)); */
    /* amplitude5 = vec4(ambient(5)); */
    /* amplitude6 = vec4(ambient(6)); */
    /* amplitude7 = vec4(ambient(7)); */
}
