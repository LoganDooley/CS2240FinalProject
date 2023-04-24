#include "profilebuffer.h"

#include <math.h>
#include <iostream>
#include "debug.h"

ProfileBuffer::ProfileBuffer(float windSpeed, int p_resolution, float z_min, int z_resolution, float unit_z):
    m_windSpeed(windSpeed),
    m_zResolution(z_resolution),
    m_pResolution(p_resolution),
    m_minZ(z_min),
    m_unitZ(unit_z)
{
    m_pbShader = ShaderLoader::createShaderProgram("Shaders/precomputeProfileBuffers.vert", "Shaders/precomputeProfileBuffers.frag");
    Debug::checkGLError();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, p_resolution, z_resolution, 0, GL_RED, GL_FLOAT, nullptr);
    Debug::checkGLError();
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
    //return 1;
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

float ProfileBuffer::cubicBump(float x) const{
    if (abs(x) >= 1){
      return 0.0f;
    }
    return x * x * (2 * abs(x) - 3) + 1;
}

// GPU IMPLEMENTATION:

void ProfileBuffer::precomputeGPU(float t, int periodicity, int integration_nodes){
    glUseProgram(m_pbShader);
    glUniform1f(glGetUniformLocation(m_pbShader, "t"), t);
    GLfloat z[m_zResolution];
    for(int i = 0; i < m_zResolution; i++){
        z[i] = m_minZ + i * m_unitZ;
    }
    glUniform1fv(glGetUniformLocation(m_pbShader, "z"), m_zResolution, z);
    glUniform1i(glGetUniformLocation(m_pbShader, "resolution"), m_pResolution);
    glUniform1i(glGetUniformLocation(m_pbShader, "periodicity"), periodicity);
    glUniform1i(glGetUniformLocation(m_pbShader, "integration_nodes"), integration_nodes);
    glUniform1f(glGetUniformLocation(m_pbShader, "windSpeed"), m_windSpeed);
    glUniform1f(glGetUniformLocation(m_pbShader, "unitZ"), m_unitZ);
}
