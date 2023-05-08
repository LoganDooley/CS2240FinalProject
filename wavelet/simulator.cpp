#include "simulator.h"
#include "../debug.h"
#include "External/imgui/imgui.h"
#include "GLWrapper/texture.h"
#include "fullscreenquad.h"
#include "glm/gtc/type_ptr.hpp"
#include "shaderloader.h"
#include "wavelet/environment.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>

Simulator::Simulator(Setting setting, std::shared_ptr<Environment> environment) :
    setting(setting), environment(environment) {
    
    if (setting.simulationResolution[0] != setting.simulationResolution[1] || 
        setting.simulationResolution[2] != 8 ||
        setting.simulationResolution[3] != 4) {
        std::cerr << "currently using unsupported resolution" << std::endl;
    }
    recomputeRanges();

    amplitude = setup3DAmplitude();
    amplitude_intermediate = setup3DAmplitude();

    fullScreenQuad = std::make_shared<FullscreenQuad>();

    recomputeFramebuffer();
    Debug::checkGLError();

    diffusionShader = ShaderLoader::createShaderProgram(
        "Shaders/waveletgrid.vert",
        "Shaders/waveletgrid_diffusion.frag"
    );
    Debug::checkGLError();

    advectionShader = ShaderLoader::createShaderProgram(
        "Shaders/waveletgrid.vert",
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
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "NUM_POS"), setting.simulationResolution[0]);
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
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    int thetaResolution = setting.simulationResolution[2];

    int attachments[8] = {
        GL_TEXTURE0, 
        GL_TEXTURE1, 
        GL_TEXTURE2, 
        GL_TEXTURE3, 
        GL_TEXTURE4, 
        GL_TEXTURE5, 
        GL_TEXTURE6, 
        GL_TEXTURE7
    };

    glViewport(0,0, setting.simulationResolution[0], setting.simulationResolution[1]);

    // advection step
    glUseProgram(advectionShader);
    glUniform1f(glGetUniformLocation(advectionShader, "time"), timeElapsed);
    glUniform1f(glGetUniformLocation(advectionShader, "deltaTime"), dt);
    advectionFBO->bind();

    fullScreenQuad->bind();

    for (int i = 0; i < thetaResolution; i++)
        amplitude[i]->bind(attachments[i]);
    environment->heightMap->bind(GL_TEXTURE8);
    environment->boundaryMap->bind(GL_TEXTURE9);
    environment->gradientMap->bind(GL_TEXTURE10);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    for (int i = 0; i < thetaResolution; i++)
        amplitude[i]->unbind(attachments[i]);

    /* // diffusion step */
    glUseProgram(diffusionShader);
    glUniform1f(glGetUniformLocation(diffusionShader, "time"), timeElapsed);
    glUniform1f(glGetUniformLocation(diffusionShader, "deltaTime"), dt);
    diffusionFBO->bind();

    for (int i = 0; i < thetaResolution; i++)
        amplitude_intermediate[i]->bind(attachments[i]);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    for (int i = 0; i < thetaResolution; i++)
        amplitude_intermediate[i]->unbind(attachments[i]);

    // this should bind the default framebuffer
    advectionFBO->unbind();

    environment->heightMap->unbind(GL_TEXTURE8);
    environment->boundaryMap->unbind(GL_TEXTURE9);
    environment->gradientMap->unbind(GL_TEXTURE10);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glUseProgram(0);

    timeElapsed += dt;
}

void Simulator::visualize(glm::ivec2 viewport) {
    ImGui::SliderInt("thetaIndex", &visualization_thetaIndex, 0, 7);

    glUseProgram(visualizationShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fullScreenQuad->bind();
    amplitude[visualization_thetaIndex]->bind(GL_TEXTURE0);

    int n = std::min(viewport.x, viewport.y);
    glViewport(0, 0, n, n);
    /* glViewport(0,0, setting.simulationResolution[0], setting.simulationResolution[1]); */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "thetaIndex"), visualization_thetaIndex);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);

    amplitude[visualization_thetaIndex]->unbind(GL_TEXTURE0);
}

void Simulator::recomputeRanges() {
    constexpr static float tau = 6.28318530718f;
    minParam = glm::vec4(-setting.size, -setting.size, 0, setting.kValues.x);
    maxParam = glm::vec4(setting.size, setting.size, tau, setting.kValues.w);
    unitParam = (maxParam - minParam) / glm::vec4(setting.simulationResolution[0], setting.simulationResolution[1], 
        setting.simulationResolution[2], setting.simulationResolution[3]);
}

void Simulator::recomputeFramebuffer() {
    int thetaResolution = setting.simulationResolution[2];

    int attachments[16] = {
        GL_COLOR_ATTACHMENT0, 
        GL_COLOR_ATTACHMENT1, 
        GL_COLOR_ATTACHMENT2, 
        GL_COLOR_ATTACHMENT3, 
        GL_COLOR_ATTACHMENT4, 
        GL_COLOR_ATTACHMENT5, 
        GL_COLOR_ATTACHMENT6, 
        GL_COLOR_ATTACHMENT7, 
        GL_COLOR_ATTACHMENT8, 
        GL_COLOR_ATTACHMENT9, 
        GL_COLOR_ATTACHMENT10, 
        GL_COLOR_ATTACHMENT11, 
        GL_COLOR_ATTACHMENT12, 
        GL_COLOR_ATTACHMENT13, 
        GL_COLOR_ATTACHMENT14, 
        GL_COLOR_ATTACHMENT15, 
    };

    advectionFBO = std::make_shared<Framebuffer>( setting.simulationResolution[0], setting.simulationResolution[1] );
    Debug::checkGLError();
    diffusionFBO = std::make_shared<Framebuffer>( setting.simulationResolution[0], setting.simulationResolution[1] );
    Debug::checkGLError();

    for (int i = 0; i < thetaResolution; i++)
        advectionFBO->attachTexture(amplitude_intermediate[i],  attachments[i]);
    for (int i = 0; i < thetaResolution; i++)
        diffusionFBO->attachTexture(amplitude[i],               attachments[i]);

    // here we dont need depth nor stencil because we're not going to do depth testing
    advectionFBO->verifyStatus();
    diffusionFBO->verifyStatus();

    reset();
}

void Simulator::reset() {
    /* GLuint clearColor[4] = {0, 0, 0, 0}; */
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glViewport(0,0, setting.simulationResolution[0], setting.simulationResolution[1]);

    glBindFramebuffer(GL_FRAMEBUFFER,advectionFBO->getHandle());
    glClear(GL_COLOR_BUFFER_BIT);
    /* glClearBufferuiv(GL_COLOR, 0, clearColor); */
    Debug::checkGLError();

    glBindFramebuffer(GL_FRAMEBUFFER,diffusionFBO->getHandle());
    glClear(GL_COLOR_BUFFER_BIT);
    /* glClearBufferuiv(GL_COLOR, 0, clearColor); */
    Debug::checkGLError();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Simulator::loadShadersWithData(GLuint shader) {
    glUseProgram(shader);
    glad_glUniform1f(glGetUniformLocation(shader, "gravity"), setting.gravity);
    glad_glUniform1f(glGetUniformLocation(shader, "surfaceTension"), setting.surfaceTension);

    glad_glUniform1i(glGetUniformLocation(shader, "NUM_POS"), setting.simulationResolution[0]);
    glad_glUniform1i(glGetUniformLocation(shader, "NUM_THETA"), setting.simulationResolution[2]);

    int thetaResolution = setting.simulationResolution[2];
    for (int i = 0; i < thetaResolution; i++) {
        std::string prop = "_Amplitude[" + std::to_string(i) + "]";
        glad_glUniform1i(glGetUniformLocation(shader, prop.c_str()), i);
    }

    glad_glUniform4fv(glGetUniformLocation(shader, "wavenumberValues"), 1, glm::value_ptr(setting.kValues));
    std::cout << glm::to_string(setting.windDirection) << std::endl;
    glad_glUniform2fv(glGetUniformLocation(shader, "windDirection"), 1, glm::value_ptr(setting.windDirection));

    glad_glUniform1i(glGetUniformLocation(shader, "_Height"), thetaResolution);
    glad_glUniform1i(glGetUniformLocation(shader, "_Gradient"), thetaResolution + 1);
    glad_glUniform1i(glGetUniformLocation(shader, "_CloseToBoundary"), thetaResolution + 2);
    glad_glUniform1f(glGetUniformLocation(shader, "waterLevel"), environment->waterHeight);

    glad_glUniform4fv(glGetUniformLocation(shader, "minParam"), 1, glm::value_ptr(minParam));
    glad_glUniform4fv(glGetUniformLocation(shader, "maxParam"), 1, glm::value_ptr(maxParam));
    glad_glUniform4fv(glGetUniformLocation(shader, "unitParam"), 1, glm::value_ptr(unitParam));
    glUseProgram(0);
}

std::vector<std::shared_ptr<Texture>> Simulator::setup3DAmplitude() {
    int thetaResolution = setting.simulationResolution[2];
    std::vector<std::shared_ptr<Texture>> textures(thetaResolution);
    for (int i = 0; i < thetaResolution; i++) {
        textures[i] = std::make_shared<Texture>();
        textures[i]->initialize2D(
                setting.simulationResolution[0], 
                setting.simulationResolution[1], 
                GL_RGBA32F, GL_RGBA, GL_FLOAT);
        textures[i]->setInterpolation(GL_LINEAR);
        textures[i]->setWrapping(GL_CLAMP_TO_BORDER);
        textures[i]->setBorderColor(glm::vec4(0,0,0,0));
    }

    return textures;
}

void Simulator::addRaindrop(){
    int thetaResolution = setting.simulationResolution[2];
    std::vector<std::shared_ptr<Texture>> textures(thetaResolution);
    for (int i = 0; i < thetaResolution; i++) {
        textures[i] = std::make_shared<Texture>();
        textures[i]->initialize2D(
                setting.simulationResolution[0],
                setting.simulationResolution[1],
                GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
        textures[i]->setInterpolation(GL_LINEAR);
        textures[i]->setWrapping(GL_CLAMP_TO_EDGE);
        /* textures[i]->setBorderColor(glm::vec4(0,0,0,0)); */
    }
}
