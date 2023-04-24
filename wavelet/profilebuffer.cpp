#include "profilebuffer.h"

#include <math.h>
#include <iostream>

ProfileBuffer::ProfileBuffer(float windSpeed):
    m_windSpeed(windSpeed)
{

}

float ProfileBuffer::value(float p) const{
    const int N = m_data.size(); // num entries
    float pi = N * p / m_period; // target "index" (between 2 indices)
    //pi = fmodf(pi, N); // loop for periodicity (array covers 1 period)

    //pi = fmaxf(0.5f, pi);
    //if(pi < 0){
        //pi += N;
    //}
    // Lerp
    int pLower = int(floor(pi));
    float wpUpper = pi - pLower; // weight towards ceiling index of pi
    pLower %= N;
    if(pLower < 0){
        pLower += N;
    }
    int pUpper = int(ceil(pi));
    pUpper %= N;
    if(pUpper < 0){
        pUpper += N;
    }

    //int pUpper = fmodf(pLower + 1, N);
    //std::cout<<"wUpper: "<<wpUpper<<" wLower: "<<1 - wpUpper<<" value: "<<wpUpper * m_data[pLower + 1] + (1-wpUpper) * m_data[pLower]<<std::endl;
    //std::cout<<"data size: "<<m_data.size()<<", pLower: "<<pLower<<std::endl;
    return wpUpper * m_data[pUpper] + (1-wpUpper) * m_data[pLower];
}

float ProfileBuffer::w(float k){
    return 1;
    constexpr float g = 9.81;
    return sqrt(k * g);
}

float ProfileBuffer::psi(float k) const {
    float A = pow(1.1, 1.5 * k);
    float B = exp(-1.8038897788076411 * pow(4, k) / pow(m_windSpeed, 4));
    return 0.139098 * sqrt(A * B);
}

float ProfileBuffer::psiBarIntegrand(float k, float p, float t){
    float waveLength = pow(2, k);
    float waveNumber = 6.28318530718 / waveLength;
    return psi(k) * cosf(waveNumber * p - w(waveNumber) * t) * waveLength;
}

// Numerically integrate equation 21 with midpoint rectangle method
float ProfileBuffer::psiBar(float p, float t, int integration_nodes, float k_min, float k_max){
    float dk = (k_max - k_min) / integration_nodes;
    float k = k_min + 0.5 * dk;

    float result = 0;
    for(int i = 0; i<integration_nodes; i++) {
        result += psiBarIntegrand(k, p, t) * dk;
        //std::cout<<"psibarIntegrand: "<<psiBarIntegrand(k, p, t)<<" dk: "<<dk<<std::endl;
        k += dk;
    }
    //std::cout<<"result: "<<result<<std::endl;
    //std::cout<<"p = "<<p<<" result = "<<result<<std::endl;
    return result;
}

void ProfileBuffer::precompute(float t, float k_min, float k_max, int resolution, int periodicity, int integration_nodes){
    m_data.resize(resolution);
    m_period = periodicity * pow(2, k_max);
    //std::cout<<m_period<<std::endl;

    for(int i = 0; i<resolution; i++){
        constexpr float tau = 6.28318530718;
        float p   = (i * m_period) / resolution;

        m_data[i] = psiBar(p, t, integration_nodes, k_min, k_max);
    }
}
