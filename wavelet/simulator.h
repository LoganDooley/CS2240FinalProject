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
    std::vector<std::shared_ptr<Texture>> getAmplitudeTextures() { return amplitude[whichPass]; }

    void addRaindrop();

private:
    int whichPass = 0; // we use 2 intermediate textures and blit between them. 
                       // So at any point, one of them is the one with outdated information.
                       // This points to the "real" one.

    float timeElapsed = 0;
    GLuint visualizationShader;
    GLuint simulationShader;

    std::shared_ptr<Environment> environment;
    std::shared_ptr<Framebuffer> simulationFBO[2];

    std::vector<std::shared_ptr<Texture>> amplitude[2];

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
