#include "simulator.h"
#include "../debug.h"
#include "External/imgui/imgui.h"
#include "GLWrapper/texture.h"
#include "fullscreenquad.h"
#include "glm/fwd.hpp"
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
    computeParameters();
    amplitude[0] = setup3DAmplitude();
    amplitude[1] = setup3DAmplitude();

    fullScreenQuad = std::make_shared<FullscreenQuad>();

    recomputeFramebuffer();
    Debug::checkGLError();

    simulationShader = ShaderLoader::createShaderProgram(
        "Shaders/waveletgrid.vert",
        "Shaders/waveletgrid_simulationStep.frag"
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

    loadShadersWithData(simulationShader);
    Debug::checkGLError();
}

Simulator::~Simulator() {
    if (simulationShader) glDeleteProgram(simulationShader);
    if (visualizationShader) glDeleteProgram(visualizationShader);
}

void Simulator::takeStep(float dt) {
    if (!dt) dt = 0.0001;
    ImGui::SliderFloat("angular diffusion scale", &setting.angularDiffusionMultiplier, 0.0f, 0.1f);
    ImGui::SliderFloat("spacial diffusion scale", &setting.spatialDiffusionMultiplier, 0.0f, 400.0f);

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
    glUseProgram(simulationShader);
    glUniform1f(glGetUniformLocation(simulationShader, "time"), timeElapsed);
    glUniform1f(glGetUniformLocation(simulationShader, "deltaTime"), dt);
    glUniform1f(glGetUniformLocation(simulationShader, "angularDiffusionMultiplier"), setting.angularDiffusionMultiplier);
    glUniform1f(glGetUniformLocation(simulationShader, "spatialDiffusionMultiplier"), setting.spatialDiffusionMultiplier);
    simulationFBO[whichPass]->bind();
    fullScreenQuad->bind();
    for (int i = 0; i < thetaResolution; i++)
        amplitude[whichPass][i]->bind(attachments[i]);
    environment->heightMap->bind(GL_TEXTURE8);
    environment->gradientMap->bind(GL_TEXTURE9);
    environment->boundaryMap->bind(GL_TEXTURE10);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    for (int i = 0; i < thetaResolution; i++)
        amplitude[whichPass][i]->unbind(attachments[i]);

    environment->heightMap->unbind(GL_TEXTURE8);
    environment->gradientMap->unbind(GL_TEXTURE9);
    environment->boundaryMap->unbind(GL_TEXTURE10);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glUseProgram(0);

    timeElapsed += dt;
    whichPass ^= 1;
}

void Simulator::visualize(glm::ivec2 viewport) {
    ImGui::SliderInt("thetaIndex", &visualization_thetaIndex, 0, 7);

    glUseProgram(visualizationShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fullScreenQuad->bind();
    amplitude[whichPass][visualization_thetaIndex]->bind(GL_TEXTURE0);

    int n = std::min(viewport.x, viewport.y);
    glViewport(0, 0, n, n);
    /* glViewport(0,0, setting.simulationResolution[0], setting.simulationResolution[1]); */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glad_glUniform1i(glGetUniformLocation(visualizationShader, "thetaIndex"), visualization_thetaIndex);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);

    amplitude[whichPass][visualization_thetaIndex]->unbind(GL_TEXTURE0);
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

    simulationFBO[0] = std::make_shared<Framebuffer>( setting.simulationResolution[0], setting.simulationResolution[1] );
    Debug::checkGLError();

    simulationFBO[1] = std::make_shared<Framebuffer>( setting.simulationResolution[0], setting.simulationResolution[1] );
    Debug::checkGLError();

    for (int i = 0; i < thetaResolution; i++)
        simulationFBO[0]->attachTexture(amplitude[1][i],  attachments[i]);
    for (int i = 0; i < thetaResolution; i++)
        simulationFBO[1]->attachTexture(amplitude[0][i],  attachments[i]);

    // here we dont need depth nor stencil because we're not going to do depth testing
    simulationFBO[0]->verifyStatus();
    simulationFBO[1]->verifyStatus();

    reset();
}

void Simulator::reset() {
    /* GLuint clearColor[4] = {0, 0, 0, 0}; */
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glViewport(0,0, setting.simulationResolution[0], setting.simulationResolution[1]);

    glBindFramebuffer(GL_FRAMEBUFFER, simulationFBO[0]->getHandle());
    glClear(GL_COLOR_BUFFER_BIT);
    /* glClearBufferuiv(GL_COLOR, 0, clearColor); */
    Debug::checkGLError();

    glBindFramebuffer(GL_FRAMEBUFFER, simulationFBO[1]->getHandle());
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
        std::string prop2 = "waveDirections[" + std::to_string(i) + "]";
        glad_glUniform2fv(glGetUniformLocation(shader, prop2.c_str()), 1, glm::value_ptr(waveDirections[i]));
        std::string prop3 = "ambient[" + std::to_string(i) + "]";
        glad_glUniform4fv(glGetUniformLocation(shader, prop3.c_str()), 1, glm::value_ptr(ambientAmplitude[i]));
    }

    glad_glUniform4fv(glGetUniformLocation(shader, "wavenumberValues"), 1, glm::value_ptr(setting.kValues));
    glad_glUniform4fv(glGetUniformLocation(shader, "angularFrequency"), 1, glm::value_ptr(angularFrequencies));
    glad_glUniform4fv(glGetUniformLocation(shader, "advectionSpeed"), 1, glm::value_ptr(advectionSpeeds));
    glad_glUniform4fv(glGetUniformLocation(shader, "dispersionSpeed"), 1, glm::value_ptr(dispersionSpeeds));
    glad_glUniform2fv(glGetUniformLocation(shader, "windDirection"), 1, glm::value_ptr(setting.windDirection));

    glad_glUniform1i(glGetUniformLocation(shader, "_Height"), 8);
    glad_glUniform1i(glGetUniformLocation(shader, "_Gradient"), 9);
    glad_glUniform1i(glGetUniformLocation(shader, "_CloseToBoundary"), 10);
    glad_glUniform1f(glGetUniformLocation(shader, "waterLevel"), environment->waterHeight);

    glad_glUniform4fv(glGetUniformLocation(shader, "minParam"), 1, glm::value_ptr(minParam));
    glad_glUniform4fv(glGetUniformLocation(shader, "maxParam"), 1, glm::value_ptr(maxParam));
    glad_glUniform4fv(glGetUniformLocation(shader, "unitParam"), 1, glm::value_ptr(unitParam));
    glUseProgram(0);
}

void Simulator::computeParameters() {
    auto angularFrequency = [this](glm::vec4 wavenumber) -> glm::vec4 {
        return glm::pow(wavenumber * setting.gravity + setting.surfaceTension * wavenumber * wavenumber * wavenumber,
                glm::vec4(0.5));
    };

    auto advectionSpeed = [this, &angularFrequency](glm::vec4 wavenumber) -> glm::vec4 {
        glm::vec4 numerator = (setting.gravity + 3 * setting.surfaceTension * wavenumber * wavenumber);
        glm::vec4 denominator = 2.0f * angularFrequency(wavenumber);
        return numerator / denominator;
    };

    auto dispersionSpeed = [this, &angularFrequency, &advectionSpeed](glm::vec4 wavenumber) -> glm::vec4 {
        // courtesy of wolfram alpha
        // https://www.wolframalpha.com/input?i=d%5E2%2Fdx%5E2%28sqrt%28ax%2Bbx%5E3%29%29
        glm::vec4 numerator =
            (-2 * setting.gravity + 6 * setting.gravity * setting.surfaceTension * wavenumber * wavenumber +
            3 * setting.surfaceTension * setting.surfaceTension * wavenumber * wavenumber * wavenumber * wavenumber);
        glm::vec4 denom = 4.0f * glm::pow(wavenumber * (setting.gravity + setting.surfaceTension * wavenumber * wavenumber), 
                glm::vec4(3 / 2.0));
        return numerator / denom;
    };

    angularFrequencies = angularFrequency(setting.kValues);
    advectionSpeeds = advectionSpeed(setting.kValues);
    dispersionSpeeds = dispersionSpeed(setting.kValues);

    waveDirections = std::vector<glm::vec2>(setting.simulationResolution[2]);
    ambientAmplitude = std::vector<glm::vec4>(setting.simulationResolution[2]);

    for (int i = 0; i < setting.simulationResolution[2]; i++) {
        float theta = (i + 0.5f) * setting.tau / setting.simulationResolution[2];
        glm::vec2 waveDirection(std::cos(theta), std::sin(theta));

        float cosTheta = glm::dot(waveDirection, glm::normalize(setting.windDirection));

        float ambientCoef = cosTheta < 0 ? 0 : cosTheta * cosTheta * 4.00f / setting.tau;

        waveDirections[i] = waveDirection;
        ambientAmplitude[i] = glm::vec4(ambientCoef);
    }
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
        textures[i]->setBorderColor(ambientAmplitude[i]);
        /* std::cout << i << " " << glm::to_string(ambientAmplitude[i]) << std::endl; */
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
