#pragma once

#include "GLWrapper/framebuffer.h"
#include "fullscreenquad.h"
#include "wavelet/waveletgrid.h"
#include <glm/glm.hpp>
#include <memory>

class Simulator {
public:
    struct GridSettings {
        float gravity = 9.81;
        float surfaceTension = 72.8 / 1000; // of water

        float size = 50;
        glm::vec2 kRange = glm::vec2(0.01, 10.0);
    };

    Simulator(std::array<int, 4> resolution, GridSettings setting);
    ~Simulator();

    void takeStep(float dt);
    void visualize(glm::ivec2 viewportSize);
    void reset();
    std::shared_ptr<Texture> getAmplitudeTexture() { return amplitude; }

private:
    float timeElapsed = 0;
    GLuint visualizationShader;
    GLuint advectionShader;
    GLuint diffusionShader;

    std::shared_ptr<Framebuffer> advectionFBO;
    std::shared_ptr<Framebuffer> diffusionFBO;
    std::shared_ptr<Texture> amplitude;
    std::shared_ptr<Texture> amplitude_intermediate;
    std::shared_ptr<FullscreenQuad> fullScreenQuad;

    int visualization_thetaIndex = 0;
    GridSettings setting;
    std::array<int, 4> resolution;
    // derived from resolution and simulation area
    glm::vec4 minParam, maxParam, unitParam;

    // to recompute minParam, maxParam, unitParam
    void recomputeRanges();
    void recomputeFramebuffer();
    void loadShadersWithData(GLuint shader);
    std::shared_ptr<Texture> setup3DAmplitude();
};
