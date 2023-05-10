#include "profilebuffer.h"

#include <math.h>
#include <iostream>
#include "debug.h"

ProfileBuffer::ProfileBuffer(float windSpeed, int p_resolution, float kMin, int integration_nodes, int kResolution):
    m_windSpeed(windSpeed),
    m_pResolution(p_resolution),
    m_kMin(kMin),
    m_integrationNodes(integration_nodes),
    m_kResolution(kResolution)
{
    m_pbShader = ShaderLoader::createShaderProgram("Shaders/precomputeProfileBuffers.vert", "Shaders/precomputeProfileBuffers.frag");
    Debug::checkGLError();
    m_texture1DShader = ShaderLoader::createShaderProgram("Shaders/texture1D.vert", "Shaders/texture1D.frag");
    Debug::checkGLError();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, p_resolution, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    Debug::checkGLError();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, p_resolution, kResolution);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // New implementation
    glGenTextures(1, &m_backgroundProfileBuffer);
    glBindTexture(GL_TEXTURE_1D, m_backgroundProfileBuffer);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 8192, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_1D, 0);

    glGenTextures(1, &m_dynamicProfileBuffer);
    glBindTexture(GL_TEXTURE_1D, m_dynamicProfileBuffer);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 8192, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_1D, 0);

    glGenFramebuffers(1, &m_profileBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_profileBufferFBO);
    glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, m_backgroundProfileBuffer, 0);
    glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_1D, m_dynamicProfileBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// GPU IMPLEMENTATION:

void ProfileBuffer::precomputeGPU(float t){
    glUseProgram(m_pbShader);
    glUniform1f(glGetUniformLocation(m_pbShader, "t"), t);
    //glUniform1i(glGetUniformLocation(m_pbShader, "pResolution"), m_pResolution);
    glUniform1f(glGetUniformLocation(m_pbShader, "windSpeed"), m_windSpeed);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_pResolution, 1);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // New Implementation
    glUseProgram(m_pbShader);
    glUniform1f(glGetUniformLocation(m_pbShader, "t"), t);
    //glUniform1i(glGetUniformLocation(m_pbShader, "pResolution"), m_pResolution);
    glUniform1f(glGetUniformLocation(m_pbShader, "windSpeed"), m_windSpeed);
    glBindFramebuffer(GL_FRAMEBUFFER, m_profileBufferFBO);
    GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);
    glViewport(0, 0, 8192, 1);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

std::vector<float> ProfileBuffer::getPeriods(){
    std::vector<float> periods;
    for(int ik = 0; ik<m_kResolution; ik++){
        float k_min = m_kMin * pow(m_integrationNodes + 1, ik);
        periods.push_back(6.28318530718 / k_min);
        //std::cout<<6.28318530718 / k_min<<std::endl;
    }
    return periods;
}

void ProfileBuffer::bindProfilebufferTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void ProfileBuffer::unbindProfilebufferTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ProfileBuffer::bindBackgroundProfileBuffer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_backgroundProfileBuffer);
}

void ProfileBuffer::unbindBackgroundProfileBuffer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
}

void ProfileBuffer::bindDynamicProfileBuffer(){
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, m_dynamicProfileBuffer);
}

void ProfileBuffer::unbindDynamicProfileBuffer(){
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, 0);
}

void ProfileBuffer::debugDraw(){
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_texture1DShader);
    Debug::checkGLError();
    bindBackgroundProfileBuffer();
    Debug::checkGLError();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    Debug::checkGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    unbindBackgroundProfileBuffer();
}
