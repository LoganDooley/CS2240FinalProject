#include "spectrum.h"

#include <math.h>

Spectrum::Spectrum(float windSpeed):
    m_windSpeed(windSpeed)
{

}

Spectrum::~Spectrum(){

}

float Spectrum::operator()(float k) const {
    float A = pow(1.1, 1.5 * k);
    float B = exp(-1.8038897788076411 * pow(4, k) / pow(m_windSpeed, 4));
    return 0.139098 * sqrt(A * B);
}
