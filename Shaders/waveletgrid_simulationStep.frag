#version 330 core

in vec2 uv;

const float tau = 6.28318530718f;

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

uniform float time;
uniform float deltaTime;

uniform vec4 minParam;
uniform vec4 maxParam;
uniform vec4 unitParam;

uniform float spatialDiffusionMultiplier = 126.5625;
uniform float angularDiffusionMultiplier = 0.025;

layout (location = 0) out vec4 outAmplitude[8];

vec2 toUV(vec2 pos) { return (pos - minParam.xy) / (maxParam.xy - minParam.xy); }
vec2 toPos(vec2 uv) { return mix(minParam.xy, maxParam.xy, uv); }
float heightDistanceToBoundary(vec2 uvPos) { return texture(_Height, uvPos).r - waterLevel; }
bool inDomain(vec2 uvPos) { return heightDistanceToBoundary(uvPos) <= 0; }

vec4 sample(vec2 uv, int itheta) {
    vec3 data = texture(_Height, uv).rgb;
    float levelSet = data.r - waterLevel;
    if (levelSet > 0) {
        uv = data.gb; // we now project out of surface
        /* return vec4(0); */

        vec2 normal = normalize(texture(_Gradient, uv)).rg;

        vec2 dir = waveDirections[itheta];

        // if not opposite to normal direction
        if (dot(dir, normal) >= 0)
            return texture(_Amplitude[itheta], uv);

        dir = reflect(dir, normal);
        float theta_reflected = atan(dir.y, dir.x);

        float itheta_reflected = (theta_reflected / tau * NUM_THETA - 0.5);
        if (itheta_reflected < 0) itheta_reflected += NUM_THETA;

        int itheta_ireflected = int(itheta_reflected);
        int itheta_ireflectedNext = itheta_ireflected+1;
        if (itheta_ireflectedNext >= NUM_THETA) itheta_ireflectedNext -= NUM_THETA;

        return vec4(1);

        /* return mix( */
        /*     texture(_Amplitude[itheta_ireflected], uv), */
        /*     texture(_Amplitude[itheta_ireflectedNext], uv), */
        /*     itheta_reflected - itheta_ireflected */
        /* ); */
    }
    // TODO: account for reflections
    return texture(_Amplitude[itheta], uv);
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

// advection  + spatial diffusion
vec4 evaluate(int itheta) {
    vec2 wavedir = waveDirections[itheta];
    vec4 wavenumber = wavenumberValues;

    vec2 pos = toPos( uv );

    float spatialResolution = unitParam.x;
    float wavenumberResolution = (wavenumber.w - wavenumber.x) / 4;
    float inverseSpatialResolutionSquared = 1/(spatialResolution * spatialResolution);

    vec4 delta = 1e-5 * spatialResolution * spatialResolution * spatialDiffusionMultiplier * 
        abs(dispersionSpeed);
    vec4 g = delta * deltaTime * inverseSpatialResolutionSquared;

    vec4 amplitude;

    for (int ik = 0; ik < NUM_K; ik++) {
        float p_prime = deltaTime * advectionSpeed[ik];

        float samplingDistance = max(unitParam.x, 2 * p_prime);

        float p[6];
        for (int pi = 0; pi < 6; pi++) {
            vec2 pi_pos = pos - samplingDistance * wavedir * (pi-2);
            vec2 pi_uv = toUV(pi_pos);
            p[pi] = sample(pi_uv, itheta)[ik];
        }

        vec4 tilda_p;
        // spatial diffusion
        for (int tilda_pi = 0; tilda_pi < 4; tilda_pi++) {
            int pi = tilda_pi + 1;
            tilda_p[tilda_pi] = (1 - 2*g[ik]) * p[pi] + g[ik] * (p[pi-1] + p[pi+1]);
        }

        p_prime /= samplingDistance;

        amplitude[ik] = guaranteeMonotonicInterpolate(tilda_p, p_prime);
    }

    return amplitude;
}

void main() {
    vec4 intermediateAmplitude[8];
    if (inDomain(uv)) {
        for (int itheta = 0; itheta < NUM_THETA; itheta++)
            intermediateAmplitude[itheta] = evaluate(itheta);
    } else {
        for (int itheta = 0; itheta < NUM_THETA; itheta++)
            intermediateAmplitude[itheta] = vec4(0);
    }

    float thetaResolution = unitParam.z;

    vec4 gamma = angularDiffusionMultiplier * advectionSpeed * unitParam.z * unitParam.z / unitParam.x;
    vec4 g = gamma * deltaTime / thetaResolution / thetaResolution;

    for (int itheta = 0; itheta < NUM_THETA; itheta++) {
        int ineg1theta = itheta - 1;
        int ipos1theta = itheta + 1;
        if (ineg1theta < 0) ineg1theta += NUM_THETA;
        if (ipos1theta >= NUM_THETA) ipos1theta -= NUM_THETA;

        outAmplitude[itheta] = (1 - 2*g) * intermediateAmplitude[itheta] + 
            g * (intermediateAmplitude[ineg1theta] + intermediateAmplitude[ipos1theta]);
    }
}
