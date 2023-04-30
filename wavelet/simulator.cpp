#include "simulator.h"
#include "../debug.h"
#include "External/imgui/imgui.h"
#include "GLWrapper/texture.h"
#include "fullscreenquad.h"
#include "glm/gtc/type_ptr.hpp"
#include "shaderloader.h"

Simulator::Simulator(std::array<int, 4> resolution, GridSettings setting) :
    setting(setting), resolution(resolution) {
    
    if (resolution[0] != resolution[1] || resolution[2] != 8 || resolution[3] != 4) {
        std::cerr << "currently using unsupported resolution" << std::endl;
    }
    recomputeRanges();

    amplitude = std::make_shared<Texture>(GL_TEXTURE0, GL_TEXTURE_3D);
    amplitude->initialize3D(resolution[0], resolution[1], resolution[2], GL_RGBA, GL_RGBA, GL_FLOAT);
    amplitude->setInterpolation(GL_LINEAR);
    amplitude->setWrapping(GL_CLAMP_TO_BORDER);
    Debug::checkGLError();

    amplitude_intermediate = std::make_shared<Texture>(GL_TEXTURE0, GL_TEXTURE_3D);
    amplitude_intermediate->initialize3D(resolution[0], resolution[1], resolution[2], GL_RGBA, GL_RGBA, GL_FLOAT);
    amplitude_intermediate->setInterpolation(GL_LINEAR);
    amplitude_intermediate->setWrapping(GL_CLAMP_TO_BORDER);
    Debug::checkGLError();

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

    glad_glUniform1i(glGetUniformLocation(visualizationShader, "_Amplitude"), 0);
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "NUM_POS"), resolution[0]);

    loadShadersWithData(advectionShader);
    loadShadersWithData(diffusionShader);
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
    glUniform1f(glGetUniformLocation(advectionShader, "deltaTime"), dt);
    amplitude->bind();
    advectionFBO->bind();

    fullScreenQuad->bind();
    glViewport(0,0,resolution[0],resolution[1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    amplitude->unbind();

    // diffusion step
    glUseProgram(diffusionShader);
    glUniform1f(glGetUniformLocation(diffusionShader, "deltaTime"), dt);
    diffusionFBO->bind();
    amplitude_intermediate->bind();

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // this should bind the default framebuffer
    advectionFBO->unbind();
    amplitude_intermediate->unbind();

    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

void Simulator::visualize() {
    ImGui::SliderInt("thetaIndex", &visualization_thetaIndex, 0, 7);

    glUseProgram(visualizationShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fullScreenQuad->bind();

    glViewport(0,0, resolution[0], resolution[1]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "thetaIndex"), visualization_thetaIndex);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
}

void Simulator::recomputeRanges() {
    minParam = glm::vec4(-setting.size, setting.size, 0, setting.kRange.x);
    maxParam = glm::vec4(-setting.size, setting.size, 0, setting.kRange.y);

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

