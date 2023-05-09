#pragma once

#include "GLWrapper/framebuffer.h"
#include "fullscreenquad.h"
#include "wavelet/environment.h"
#include "wavelet/setting.h"
#include "wavelet/waveletgrid.h"
#include <glm/glm.hpp>
#include <memory>

class Simulator {
public:
    Simulator(Setting settings, std::shared_ptr<Environment> environment);
    ~Simulator();

    void takeStep(float dt);
    void visualize(glm::ivec2 viewportSize);
    void reset();
    std::vector<std::shared_ptr<Texture>> getAmplitudeTextures() { return amplitude; }

    void addRaindrop();

private:
    float timeElapsed = 0;
    GLuint visualizationShader;
    GLuint advectionShader;
    GLuint diffusionShader;

    std::shared_ptr<Environment> environment;
    std::shared_ptr<Framebuffer> advectionFBO;
    std::shared_ptr<Framebuffer> diffusionFBO;

    std::vector<std::shared_ptr<Texture>> amplitude;
    std::vector<std::shared_ptr<Texture>> amplitude_intermediate;

    std::shared_ptr<FullscreenQuad> fullScreenQuad;

    const Setting setting;
    int visualization_thetaIndex = 0;
    // derived from resolution and simulation area
    glm::vec4 minParam, maxParam, unitParam;

    // all precomputed data on the cpu to be loaded onto the cpu. 
    // This only happens once, so hopefully it's not too expensive.
    std::vector<glm::vec2> waveDirections;
    std::vector<glm::vec4> ambientAmplitude;
    glm::vec4 angularFrequencies;
    glm::vec4 advectionSpeeds;
    glm::vec4 dispersionSpeeds;

    // to recompute minParam, maxParam, unitParam
    void computeParameters();
    void recomputeRanges();
    void recomputeFramebuffer();
    void loadShadersWithData(GLuint shader);
    std::vector<std::shared_ptr<Texture>> setup3DAmplitude();
};
