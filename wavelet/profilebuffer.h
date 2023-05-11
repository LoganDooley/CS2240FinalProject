#pragma once

#include <array>
#include <vector>
#include <math.h>

#include "shaderloader.h"

class ProfileBuffer
{
public:
    ProfileBuffer(){};
    ProfileBuffer(float windSpeed, int p_resolution, float kMin, int integration_nodes, int k_resolution);
    ~ProfileBuffer(){};

    int getKResolution() const{
        return m_kResolution;
    }

    int getIntegrationNodes() const{
        return m_integrationNodes;
    }

public:
    void precomputeGPU(float t);
    std::vector<float> getPeriods();
    void bindProfilebufferTexture();
    void unbindProfilebufferTexture();

    void bindBackgroundProfileBuffer();
    void unbindBackgroundProfileBuffer();
    void bindDynamicProfileBuffer();
    void unbindDynamicProfileBuffer();
    void debugDraw();

private:
    GLuint m_pbShader;
    GLuint m_texture1DShader;
    GLuint m_backgroundProfileBuffer;
    GLuint m_dynamicProfileBuffer;
    GLuint m_backgroundProfileBufferFBO;
    GLuint m_dynamicProfileBufferFBO;
    int m_kResolution;
    int m_pResolution;
    float m_kMin;
    int m_integrationNodes;

    float m_windSpeed = 1;
    float m_period = 0;
    std::vector<float> m_data = {};
};
