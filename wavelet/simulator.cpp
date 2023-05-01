#include "simulator.h"
#include "../debug.h"
#include "External/imgui/imgui.h"
#include "GLWrapper/texture.h"
#include "fullscreenquad.h"
#include "glm/gtc/type_ptr.hpp"
#include "shaderloader.h"
#include <glm/ext.hpp>

Simulator::Simulator(std::array<int, 4> resolution, GridSettings setting) :
    setting(setting), resolution(resolution) {
    
    if (resolution[0] != resolution[1] || resolution[2] != 8 || resolution[3] != 4) {
        std::cerr << "currently using unsupported resolution" << std::endl;
    }
    recomputeRanges();

    amplitude = setup3DAmplitude();
    amplitude_intermediate = setup3DAmplitude();

    // ambient amplitude as border lol, might just work for now
    amplitude->setBorderColor(glm::vec4(0,0,0,0));

    fullScreenQuad = std::make_shared<FullscreenQuad>();

    recomputeFramebuffer();

    diffusionShader = ShaderLoader::createShaderProgram(
        "Shaders/waveletgrid.vert",
        "Shaders/waveletgrid.geom",
        "Shaders/waveletgrid_diffusion.frag"
    );
    Debug::checkGLError();

    advectionShader = ShaderLoader::createShaderProgram(
        "Shaders/waveletgrid.vert",
        "Shaders/waveletgrid.geom",
        "Shaders/waveletgrid_advection.frag"
    );
    Debug::checkGLError();

    visualizationShader = ShaderLoader::createShaderProgram(
        "Shaders/waveletgrid_visualizer.vert",
        "Shaders/waveletgrid_visualizer.frag"
    );
    Debug::checkGLError();

    glUseProgram(visualizationShader);
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "_Amplitude"), 0);
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "NUM_POS"), resolution[0]);
    glUseProgram(0);
    Debug::checkGLError();

    loadShadersWithData(advectionShader);
    Debug::checkGLError();
    loadShadersWithData(diffusionShader);
    Debug::checkGLError();
}

Simulator::~Simulator() {
    if (diffusionShader) glDeleteProgram(diffusionShader);
    if (advectionShader) glDeleteProgram(advectionShader);
    if (visualizationShader) glDeleteProgram(visualizationShader);
}

void Simulator::takeStep(float dt) {
    glDisable(GL_DEPTH_TEST);

    // advection step
    glUseProgram(advectionShader);
    glUniform1f(glGetUniformLocation(advectionShader, "time"), timeElapsed);
    glUniform1f(glGetUniformLocation(advectionShader, "deltaTime"), dt);
    advectionFBO->bind();
    amplitude->bind();
    fullScreenQuad->bind();

    glViewport(0,0,resolution[0],resolution[1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    amplitude->unbind();

    // diffusion step
    glUseProgram(diffusionShader);
    glUniform1f(glGetUniformLocation(diffusionShader, "time"), timeElapsed);
    glUniform1f(glGetUniformLocation(diffusionShader, "deltaTime"), dt);
    diffusionFBO->bind();
    amplitude_intermediate->bind();

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    amplitude_intermediate->unbind();

    // this should bind the default framebuffer
    advectionFBO->unbind();

    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);

    timeElapsed += dt;
}

void Simulator::visualize(glm::ivec2 viewport) {
    ImGui::SliderInt("thetaIndex", &visualization_thetaIndex, 0, 7);

    glUseProgram(visualizationShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fullScreenQuad->bind();
    amplitude->bind();

    int n = std::min(viewport.x, viewport.y);
    /* glViewport(0, 0, n, n); */
    glViewport(0,0, resolution[0], resolution[1]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "thetaIndex"), visualization_thetaIndex);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
}

void Simulator::recomputeRanges() {
    constexpr static float tau = 6.28318530718f;
    minParam = glm::vec4(-setting.size, -setting.size, 0, setting.kRange.x);
    maxParam = glm::vec4(setting.size, setting.size, tau, setting.kRange.y);

    unitParam = (maxParam - minParam) / glm::vec4(resolution[0], resolution[1], resolution[2], resolution[3]);
}

void Simulator::recomputeFramebuffer() {
    advectionFBO = std::make_shared<Framebuffer>( resolution[0], resolution[1] );
    diffusionFBO = std::make_shared<Framebuffer>( resolution[0], resolution[1] );

    advectionFBO->attachTexture(amplitude_intermediate, GL_COLOR_ATTACHMENT0, true);
    diffusionFBO->attachTexture(amplitude, GL_COLOR_ATTACHMENT0, true);

    // here we dont need depth nor stencil because we're not going to do depth testing
    advectionFBO->verifyStatus();
    diffusionFBO->verifyStatus();

    GLuint clearColor[4] = {0, 0, 0, 0};

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,advectionFBO->getHandle());
    glClearBufferuiv(GL_COLOR, 0, clearColor);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,diffusionFBO->getHandle());
    glClearBufferuiv(GL_COLOR, 0, clearColor);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Simulator::loadShadersWithData(GLuint shader) {
    glUseProgram(shader);
    glad_glUniform1f(glGetUniformLocation(shader, "gravity"), setting.gravity);
    glad_glUniform1f(glGetUniformLocation(shader, "surfaceTension"), setting.surfaceTension);

    glad_glUniform1i(glGetUniformLocation(shader, "NUM_POS"), resolution[0]);
    glad_glUniform1i(glGetUniformLocation(shader, "NUM_THETA"), resolution[2]);

    glad_glUniform1i(glGetUniformLocation(shader, "_Amplitude"), 0);

    glad_glUniform4fv(glGetUniformLocation(shader, "minParam"), 1, glm::value_ptr(minParam));
    glad_glUniform4fv(glGetUniformLocation(shader, "maxParam"), 1, glm::value_ptr(maxParam));
    glad_glUniform4fv(glGetUniformLocation(shader, "unitParam"), 1, glm::value_ptr(unitParam));
    glUseProgram(0);
}

std::shared_ptr<Texture> Simulator::setup3DAmplitude() {
    std::shared_ptr<Texture> amp = std::make_shared<Texture>(GL_TEXTURE0, GL_TEXTURE_3D);
    amp ->initialize3D(resolution[0], resolution[1], resolution[2], GL_RGBA, GL_RGBA, GL_FLOAT);
    amp->setInterpolation(GL_LINEAR);
    amp->setWrapping(GL_CLAMP_TO_BORDER);
    amp->bind();

    Debug::checkGLError();
    return amp;
}

