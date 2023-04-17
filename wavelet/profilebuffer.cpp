#include "profilebuffer.h"

#include <math.h>

ProfileBuffer::ProfileBuffer(float windSpeed):
    m_windSpeed(windSpeed)
{

}

float ProfileBuffer::density(float k) const {
    float A = pow(1.1, 1.5 * k);
    float B = exp(-1.8038897788076411 * pow(4, k) / pow(m_windSpeed, 4));
    return 0.139098 * sqrt(A * B);
}

void ProfileBuffer::precompute(float time, float k_min, float k_max, int resolution, int periodicity, int integration_nodes){
    m_data.resize(resolution);
    m_period = periodicity * pow(2, k_max);

    for(int i = 0; i<resolution; i++){
        constexpr float tau = 6.28318530718;
        float p   = (i * m_period) / resolution;

    }
}
