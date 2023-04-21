#include "waveletgrid.h"
#include "wavelet/amplitude.h"
#include <assert.h>
#include <iterator>
#include <math.h>
#include <glm/vec2.hpp>
#include "mathutil.h"
#include <tuple>

// wavelet grid
WaveletGrid::WaveletGrid(std::array<unsigned int, 4> resolution)
    : m_resolution(resolution) {

        glm::vec4 resolutionVec(resolution[Parameter::X], resolution[Parameter::Y], resolution[Parameter::THETA],
                resolution[Parameter::K]);

        m_minParam = glm::vec4(-settings.size, -settings.size, 0, settings.k_range[0]);
        m_maxParam = glm::vec4(settings.size, settings.size, tau, settings.k_range[1]);
        m_unitParam = (m_maxParam - m_minParam) / resolutionVec;

        m_profileBuffer = std::make_unique<ProfileBuffer>(5);
}

void WaveletGrid::takeStep(float dt){
    time += dt;

    m_profileBuffer->precompute(time, m_minParam[Parameter::K], m_maxParam[Parameter::K]);
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

glm::vec2 WaveletGrid::getWaveDirection(glm::vec4 pos) const {
  float theta = pos[Parameter::THETA];
  return glm::vec2(cosf(theta), sinf(theta));
}

void WaveletGrid::advectionStep(float deltaTime) {
    for (unsigned int i_x = 0; i_x < amplitudes.getResolution(Parameter::X); i_x++) {
        for (unsigned int i_y = 0; i_y < amplitudes.getResolution(Parameter::Y); i_y++) {
            std::array<unsigned int, 2> i_xy = {i_x, i_y};
            glm::vec2 pos = getPositionAtIndex(i_xy);
            // we need not compute the advection for points outside of the domain.
            if (m_environment.inDomain(pos)) {
                for (unsigned int i_theta = 0; i_theta < amplitudes.getResolution(Parameter::THETA); i_theta++) {
                    for (unsigned int i_k = 0; i_k < amplitudes.getResolution(Parameter::K); i_k++) {
                        glm::vec4 pos = getPositionAtIndex({i_x, i_y, i_theta, i_k});
                        glm::vec2 kb = getWaveDirection(pos);
                        // ought also use advectionSpeed here? representing omega in equation 17?
                        float omega = advectionSpeed(i_k);
                        glm::vec4 lagrangianPos = pos;
                        lagrangianPos[Parameter::X] -= deltaTime * kb[0] * omega;
                        lagrangianPos[Parameter::Y] -= deltaTime * kb[1] * omega;
                        // handle reflection over terrain.
                        lagrangianPos = getReflected(lagrangianPos);
                        amplitudes_nxt(i_x, i_y, i_theta, i_k) = lookup_interpolated_amplitude(
                                        lagrangianPos[Parameter::X], lagrangianPos[Parameter::Y],
                                        i_theta, i_k);
                    }
                }
            }
        }
    }
    std::swap(amplitudes, amplitudes_nxt);
}

void WaveletGrid::diffusionStep(float deltaTime) {
    float spacialResolution = m_unitParam[Parameter::X];


    for (unsigned int i_x = 0; i_x < amplitudes.getResolution(Parameter::X); i_x++)
    for (unsigned int i_y = 0; i_y < amplitudes.getResolution(Parameter::Y); i_y++) {

        std::array<unsigned int, 2> i_xy = {i_x, i_y};
        float distanceToBoundary = m_environment.levelSet(getPositionAtIndex(i_xy));

        for (unsigned int i_theta = 0; i_theta < amplitudes.getResolution(Parameter::THETA); i_theta++)
        for (unsigned int i_k = 0; i_k < amplitudes.getResolution(Parameter::K); i_k++) {
            glm::vec4 pos = getPositionAtIndex({i_x, i_y, i_theta, i_k});
            float wavenumber = pos[K];
            float theta = pos[THETA];

            // TODO: precompute this
            glm::vec2 k_hat(cos(theta), sin(theta));

            bool atLeast2AwayFromBoundary = distanceToBoundary >= 4 * spacialResolution;
            
            float amplitude = amplitudes(i_x, i_y, i_theta, i_k);

            if (atLeast2AwayFromBoundary) {
                // found on bottom of page 6
                float delta = 1e-5 * spacialResolution * spacialResolution *
                    (m_unitParam[Parameter::K] * m_unitParam[Parameter::K]) * dispersionSpeed(wavenumber);

                // found on bottom of page 6
                float gamma = 0.025 * advectionSpeed(wavenumber) * m_unitParam[Parameter::THETA] *
                    m_unitParam[Parameter::THETA] / spacialResolution;

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
    }

    std::swap(amplitudes, amplitudes_nxt);
}

float WaveletGrid::amplitude(std::array<float, 4> pos) const{
    glm::vec4 indexPos = posToIdx(glm::vec4(pos[0], pos[1], pos[2], pos[3]));

    std::function<float(int,int,int,int)> f = std::bind(&WaveletGrid::lookup_amplitude, this, 
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    return Math::interpolate4D(indexPos[Parameter::X], indexPos[Parameter::Y], indexPos[Parameter::THETA], indexPos[Parameter::K], 
        f, m_environment
    );
}

float WaveletGrid::surfaceAtPoint(glm::vec2 pos) {
    float height = 0;

    int DIR_NUM = m_resolution[Parameter::THETA];
    for(int ik = 0; ik < m_resolution[Parameter::K]; ik++){
        float da = tau/DIR_NUM;

        for(int itheta = 0; itheta < DIR_NUM; itheta++) {
            float angle = itheta * da * tau;
            glm::vec2 kdir = glm::vec2(cosf(angle), sinf(angle));
            float kdir_x = glm::dot(kdir, pos);

            height += da * lookup_interpolated_amplitude(pos.x, pos.y, angle, ik) * m_profileBuffer->value(kdir_x);
        }
    }

    return height;
}

float WaveletGrid::idxToPos(const unsigned int idx, Parameter p) const{
    return m_minParam[p] + (idx + 0.5) * m_unitParam[p];
}

bool WaveletGrid::outOfBounds(glm::vec2 pos) const {
    for (int dim = 0; dim < 2; dim++)
        if ( m_minParam[dim] > pos[dim] || m_maxParam[dim] < pos[dim] )
            return false;
    return true;
}

std::tuple<float,float> WaveletGrid::posToIdx(float x, float y) const {
    // we are doing inverse of idxToPos
    return { (x - m_minParam.x) / m_unitParam.x - 0.5, (y - m_minParam.y) / m_unitParam.y - 0.5};
}

glm::vec4 WaveletGrid::posToIdx(glm::vec4 pos4) const {
    return (pos4 - m_minParam) / m_unitParam - glm::vec4(0.5);
}

glm::vec4 WaveletGrid::getPositionAtIndex(std::array<unsigned int, 4> index) const {
    glm::vec4 indexVec(index[0], index[1], index[2], index[3]);
    return m_minParam + (indexVec + glm::vec4(0.5)) * m_unitParam;
}

glm::vec2 WaveletGrid::getPositionAtIndex(std::array<unsigned int, 2> index) const {
    return glm::vec2(idxToPos(index[0], Parameter::X), idxToPos(index[1], Parameter::Y));
}

float WaveletGrid::ambientAmplitude(float x, float y, int i_theta, int i_k) const {
    // TODO: figure out this ambient term
    return 0.0f;
}

glm::vec4 WaveletGrid::getReflected(glm::vec4 pos) const {
    glm::vec2 posxy = glm::vec2(pos);

    float distanceToBoundary = m_environment.levelSet(posxy);
    // 0 is on the boundary and we only simulate anything below 0, so we don't need
    // to reflect if this is higher or equal
    if (distanceToBoundary >= 0) {
        return pos;
    }

    glm::vec2 normal = m_environment.levelSetGradient(posxy);
    float theta = pos[Parameter::THETA];
    glm::vec2 wavedirection = glm::vec2(std::cos(theta), std::sin(theta));

    glm::vec2 reflectedPos = posxy - 2.f * normal * distanceToBoundary;
    glm::vec2 reflectedDirection = wavedirection - 2.f * (glm::dot(normal, wavedirection) * normal);

    float reflectedTheta = std::atan2(reflectedDirection.y, reflectedDirection.x);

    // this should be in the domain (like right on the edge)
    assert(m_environment.inDomain(pos));

    return glm::vec4(reflectedPos.x, reflectedPos.y, reflectedTheta, pos[Parameter::K]);
}

float WaveletGrid::lookup_interpolated_amplitude(float x, float y, int i_theta, int i_k) {
    if (outOfBounds(glm::vec2(x,y))) return ambientAmplitude(x,y,i_theta,i_k);

    // convert (x,y) into index positions
    std::tie(x,y) = posToIdx(x,y);

    auto f = [this, i_theta, i_k](int i_x, int i_y) -> float {
        if (i_x < 0 || i_x >= m_resolution[Parameter::X] || i_y < 0 || i_y >= m_resolution[Y]) {
            // we need an amplitude for a point outside of the simulation box
            return ambientAmplitude(idxToPos(i_x, Parameter::X), idxToPos(i_y, Parameter::Y), i_theta, i_k);
        }

        return amplitudes(i_x, i_y, i_theta, i_k);
    };

    return Math::interpolate2D(x, y, f);
}

float WaveletGrid::lookup_amplitude(int i_x, int i_y, int i_theta, int i_k) const {
    i_theta = (i_theta + m_resolution[Parameter::THETA]) % m_resolution[Parameter::THETA];

    if (i_k < 0 || i_k >= m_resolution[Parameter::K])
        return 0.0f;

    if (i_x < 0 || i_x >= m_resolution[Parameter::X] || i_y < 0 || i_y >= m_resolution[Y])
        // we need an amplitude for a point outside of the simulation box
        return ambientAmplitude(idxToPos(i_x, Parameter::X), idxToPos(i_y, Parameter::Y), i_theta, i_k);

    return amplitudes(i_x, i_y, i_theta, i_k);
};
