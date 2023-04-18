#include "profilebuffer.h"

#include <math.h>

ProfileBuffer::ProfileBuffer(float windSpeed):
    m_windSpeed(windSpeed)
{

}

float ProfileBuffer::w(float k) const{
    constexpr float g = 9.81;
    return sqrt(k * g);
}

float ProfileBuffer::psi(float k) const {
    float A = pow(1.1, 1.5 * k);
    float B = exp(-1.8038897788076411 * pow(4, k) / pow(m_windSpeed, 4));
    return 0.139098 * sqrt(A * B);
}

float ProfileBuffer::psiBarIntegrand(float k, float p, float t){
    return psi(k) * cosf(k * p - w(k) * t) * k;
}

float ProfileBuffer::psiBar(float p, float t, int integration_nodes, float k_min, float k_max){
    float dk = (k_max - k_min) / integration_nodes;
    float k = k_min + 0.5 * dk;

    float result = dk * psiBarIntegrand(k, p, t);
    for(int i = 1; i<integration_nodes; i++) {
        k += dk;
        result += dk * psiBarIntegrand(k, p, t);
    }

    return result;
}

void ProfileBuffer::precompute(float t, float k_min, float k_max, int resolution, int periodicity, int integration_nodes){
    m_data.resize(resolution);
    m_period = periodicity * pow(2, k_max);

    for(int i = 0; i<resolution; i++){
        constexpr float tau = 6.28318530718;
        float p   = (i * m_period) / resolution;

        m_data[i] = psiBar(p, t, integration_nodes, k_min, k_max);;
    }
}
