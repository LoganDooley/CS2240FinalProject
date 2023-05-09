#version 330 core

in vec2 uv;

// DIMENSIONS
const int NUM_K = 4; // this must not be higher than 4
uniform int NUM_THETA = 8;
uniform int NUM_POS = 2048;

uniform vec4 wavenumberValues;
uniform vec4 angularFrequency;
uniform vec4 advectionSpeed;
uniform vec4 dispersionSpeed;

uniform vec2 waveDirections[8];
uniform vec4 ambient[8];
uniform vec2 windDirection;

uniform sampler2D _Amplitude[8];
uniform sampler2D _Height;
uniform sampler2D _Gradient;
uniform sampler2D _CloseToBoundary;
uniform float waterLevel = 0.641;

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

float heightDistanceToBoundary(vec2 uvPos) {
    return texture(_Height, uvPos).r - waterLevel;
}

bool inDomain(vec2 uvPos) {
    return heightDistanceToBoundary(uvPos) <= 0;
}

vec4 amplitude(vec2 uv, float thetaUV) {
    float thetaTexCoord = (thetaUV * NUM_THETA) - 0.5;
    if (thetaTexCoord < 0) thetaTexCoord += NUM_THETA;

    int itheta_simulated = int(thetaTexCoord);
    int itheta_simulated_p1 = (itheta_simulated + 1) % NUM_THETA;

    vec4 v0 = texture(_Amplitude[itheta_simulated], uv);
    vec4 v1 = texture(_Amplitude[itheta_simulated_p1], uv);

    float t = thetaTexCoord - itheta_simulated;

    return mix(v0, v1, vec4(t));
}

vec2 posToUV(vec2 pos) {
    return (pos - minParam.xy) / (maxParam.xy - minParam.xy);
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
    if (ix < 0 || ix >= NUM_POS || iy < 0 || iy >= NUM_POS)
        return ambient[itheta][izeta];
    return texelFetch(_Amplitude[itheta], ivec2(ix, iy), 0)[izeta];
}

// we can break this into 2 shaders and instead of doing 16 texture fetches, we only
// do 8 but i dont know if that's that much better
float interpolate2D(float x, float y, int itheta, int izeta) {
    if (x < 0 || x >= NUM_POS || y < 0 || y >= NUM_POS)
        return ambient[itheta][izeta];

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
        w[dy] = niy < 0 || niy >= NUM_POS ? ambient[itheta][izeta] 
            : guaranteeMonotonicInterpolate(v, x - ix);
    }

    return guaranteeMonotonicInterpolate(w, y-iy);
}

vec4 evaluate(int thetaIndex) {
    vec2 pos = mix(minParam.xy, maxParam.xy, uv);

    vec2 wavedirection = waveDirections[thetaIndex];

    vec4 amplitude;

#pragma openNV (unroll all)
    for (int zetaIndex = 0; zetaIndex < NUM_K; zetaIndex++) {
        // since we're using zeta
        vec2 nxtPos = pos - deltaTime * wavedirection * advectionSpeed[zetaIndex];

        vec2 nxtPosUV = posToUV(nxtPos);
        float distanceToBoundary = heightDistanceToBoundary(nxtPosUV);

        vec2 nxtPosTexCoord = nxtPosUV * NUM_POS - 0.5;

        float interpolatedAmplitude =
            interpolate2D(nxtPosTexCoord.x, nxtPosTexCoord.y, thetaIndex, zetaIndex);

        /* vec2 uvPos = posToUV(pos); */
        /* float distanceToBoundary = heightDistanceToBoundary(uvPos); */
        /* // still in water, no reflection */
        /* if (distanceToBoundary <= 0) */
        /*     return vec3(pos, theta); */

        /* vec2 normal = texture(_Gradient, uvPos).rg; */
        /* vec2 wavedirection = vec2(cos(theta), sin(theta)); */

        /* vec2 reflectedPos = pos - 2.0 * normal * distanceToBoundary; */
        /* vec2 reflectedDirection = wavedirection - 2.0 * dot(normal, wavedirection) * normal; */

        /* float reflectedTheta = atan(reflectedDirection.y, reflectedDirection.x); */

        /* return vec3(reflectedPos, reflectedTheta); */

        // ambient amplitude if outside grid

        amplitude[zetaIndex] = interpolatedAmplitude;
    }

    /* return texture(_Amplitude[thetaIndex], vec2(-1,-1)); */
    return amplitude;
}

void main() {
    /* if (inDomain(uv)) { */
        amplitude0 = evaluate(0);
        amplitude1 = evaluate(1);
        amplitude2 = evaluate(2);
        amplitude3 = evaluate(3);
        amplitude4 = evaluate(4);
        amplitude5 = evaluate(5);
        amplitude6 = evaluate(6);
        amplitude7 = evaluate(7);
    /* } else { */
    /*     amplitude0 = vec4(0); */
    /*     amplitude1 = vec4(0); */
    /*     amplitude2 = vec4(0); */
    /*     amplitude3 = vec4(0); */
    /*     amplitude4 = vec4(0); */
    /*     amplitude5 = vec4(0); */
    /*     amplitude6 = vec4(0); */
    /*     amplitude7 = vec4(0); */
    /* } */

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
}
