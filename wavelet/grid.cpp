#include "grid.h"
#include "wavelet/amplitude.h"
#include <assert.h>
#include <iterator>
#include <math.h>
#include <glm/vec2.hpp>

// wavelet grid
WaveletGrid::WaveletGrid(std::array<unsigned int, 4> resolution)
    : m_minParam(-size, -size, 0, minZeta), m_maxParam(size, size, tau, maxZeta),
    m_resolution(resolution) {

        glm::vec4 resolutionVec(resolution[Parameter::X], resolution[Parameter::Y], resolution[Parameter::THETA],
                resolution[Parameter::K]);
        m_pixelParam = (m_maxParam - m_minParam) / resolutionVec;
    }

float WaveletGrid::angularFrequency(float wavenumber) {
    return sqrt(wavenumber * gravity +
            surfaceTension * wavenumber * wavenumber * wavenumber);
}

float WaveletGrid::advectionSpeed(float wavenumber) {
    float numerator = (gravity + 3 * surfaceTension * wavenumber * wavenumber);
    float denominator = 2 * angularFrequency(wavenumber);
    assert(denominator);
    return numerator / denominator;
}

float WaveletGrid::dispersionSpeed(float wavenumber) {
    // courtesy of wolfram alpha 
    // https://www.wolframalpha.com/input?i=d%5E2%2Fdx%5E2%28sqrt%28ax%2Bbx%5E3%29%29
    float numerator =
        (-2 * gravity + 6 * gravity * surfaceTension * wavenumber * wavenumber +
         3 * surfaceTension * surfaceTension * wavenumber * wavenumber * wavenumber * wavenumber);
    float denom = 4 * pow(wavenumber * (gravity + surfaceTension * wavenumber * wavenumber), 3 / 2);
    assert(denom);
    return numerator / denom;
}

void WaveletGrid::advectionStep(float deltaTime) {

}

void WaveletGrid::diffusionStep(float deltaTime) {
    float spacialResolution = m_pixelParam[Parameter::X] * m_pixelParam[Parameter::Y];

    auto lookup_amplitude = [this](int i_x, int i_y, int i_theta, int i_k) {
        i_theta = (i_theta + m_resolution[Parameter::THETA]) % m_resolution[Parameter::THETA];

        if (i_k < 0 || i_k >= m_resolution[Parameter::K])
            return 0.0f;

        if (i_x < 0 || i_x >= m_resolution[Parameter::X] || i_y < 0 || i_y >= m_resolution[Y]) {
            // we need an amplitude for a point outside of the simulation box
            // TODO: actuallly return the default amplitude for wavevector specified by i_theta and i_k
            return 0.0f;
        }

        return amplitudes(i_x, i_y, i_theta, i_k);
    };

    for (unsigned int i_x = 0; i_x < amplitudes.getResolution(Parameter::X); i_x++)
    for (unsigned int i_y = 0; i_y < amplitudes.getResolution(Parameter::Y); i_y++)
    for (unsigned int i_theta = 0; i_theta < amplitudes.getResolution(Parameter::THETA); i_theta++)
    for (unsigned int i_k = 0; i_k < amplitudes.getResolution(Parameter::K); i_k++) {
        glm::vec4 pos = getPositionAtIndex({i_x, i_y, i_theta, i_k});
        float wavenumber = pos[K];
        float theta = pos[THETA];

        // TODO: precompute this
        glm::vec2 k_hat(cos(theta), sin(theta));

        // TODO: write code to determine if a position is at least 2 away from
        // boundary
        bool atLeast2AwayFromBoundary = true;
        
        float amplitude = amplitudes(i_x, i_y, i_theta, i_k);

        if (atLeast2AwayFromBoundary) {
            // found on bottom of page 6
            float delta = 1e-5 * spacialResolution * spacialResolution *
                (m_pixelParam[Parameter::K] * m_pixelParam[Parameter::K]) * dispersionSpeed(wavenumber);

            // found on bottom of page 6
            float gamma = 0.025 * advectionSpeed(wavenumber) * m_pixelParam[Parameter::THETA] *
                m_pixelParam[Parameter::THETA] / spacialResolution;

            int h = 1; // step size

            // caching some values common to the calculations below
            float inverseH2 = 1.0f / (h*h);
            float inverse2H = 1.0f / (2*h);

            float lookup_xh_y_theta_k = lookup_amplitude(i_x + h, i_y, i_theta, i_k);
            float lookup_xnegh_y_theta_k = lookup_amplitude(i_x - h, i_y, i_theta, i_k);

            float lookup_x_yh_theta_k = lookup_amplitude(i_x, i_y + h, i_theta, i_k);
            float lookup_x_ynegh_theta_k = lookup_amplitude(i_x, i_y - h, i_theta, i_k);

            // we are actually using a step size of h/2 here
            // use central difference to obtain d2A / dtheta^2 numerically
            float secondPartialDerivativeWRTtheta = (lookup_amplitude(i_x, i_y, i_theta + h, i_k) + lookup_amplitude(i_x, i_y, i_theta - h, i_k) - 2 * amplitude) * inverseH2;

            // use central difference to obtain (k dot V_x)
            float partialDerivativeWRTX = (lookup_xh_y_theta_k - lookup_xnegh_y_theta_k) * inverse2H;
            float partialDerivativeWRTY = (lookup_x_yh_theta_k - lookup_x_ynegh_theta_k) * inverse2H;
            float directionalDerivativeWRTK = glm::dot(k_hat, glm::vec2(partialDerivativeWRTX, partialDerivativeWRTY));

            // central difference to obtain (k dot V_x)^2
            float secondPartialDerivativeWRTX = (lookup_xh_y_theta_k + lookup_xnegh_y_theta_k - 2 * amplitude) * inverseH2;
            float secondPartialDerivativeWRTY = (lookup_x_yh_theta_k + lookup_x_ynegh_theta_k - 2 * amplitude) * inverseH2;
            float secondDirectionalDerivativeWRTK = glm::dot(k_hat * k_hat, glm::vec2(secondPartialDerivativeWRTX, secondPartialDerivativeWRTY));

            // equation 18
            float derivativeWRTt = -advectionSpeed(wavenumber) * directionalDerivativeWRTK + delta * secondDirectionalDerivativeWRTK + gamma * secondPartialDerivativeWRTtheta;

            amplitude += derivativeWRTt * deltaTime;
        }

        amplitudes_nxt(i_x, i_y, i_theta, i_k) = amplitude;
    }

    std::swap(amplitudes, amplitudes_nxt);
}

glm::vec3 WaveletGrid::surfaceAtPoint(glm::vec2 pos) const {
    glm::vec3 finalPos = glm::vec3(0, 0, 0);

    for(int ik = 0; ik < m_resolution[Parameter::K]; ik++){
        float k = idxToPos(ik, Parameter::K);
        auto& profile = m_profileBuffers[k];

        int DIR_NUM = m_resolution[Parameter::THETA];
        int N = 4 * DIR_NUM;
        float da = 1.f/N;
        float dx = DIR_NUM * tau / N;

        for(float a = 0; a < 1; a += da) {
            float angle = a * tau;
            glm::vec2 kdir = glm::vec2(cosf(angle), sinf(angle));
            float kdir_x = glm::dot(kdir, pos);

        }
    }
}

float WaveletGrid::idxToPos(const unsigned int idx, Parameter p) const{
    return m_minParam[p] + (idx + 0.5) * m_resolution[p];
}

glm::vec4 WaveletGrid::getPositionAtIndex(std::array<unsigned int, 4> index) {
    glm::vec4 indexVec(index[0], index[1], index[2], index[3]);
    return m_minParam + (indexVec + glm::vec4(0.5)) * m_pixelParam;
}
