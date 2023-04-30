#include "simulator.h"
#include "../debug.h"
#include "GLWrapper/texture.h"

Simulator::Simulator(std::array<int, 4> resolution, GridSettings setting) :
    setting(setting), resolution(resolution) {
    recomputeRanges();

    amplitude = std::make_shared<Texture>(GL_TEXTURE0, GL_TEXTURE_3D);
    amplitude->initialize3D(resolution[0], resolution[1], resolution[2], GL_RGBA, GL_RGBA, GL_FLOAT);
    amplitude->setInterpolation(GL_LINEAR);
    amplitude->setWrapping(GL_CLAMP_TO_BORDER);

    // ambient amplitude as border lol, might just work for now
    amplitude->setBorderColor(glm::vec4(0,0,0,0));

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
}

Simulator::~Simulator() {
}

void Simulator::takeStep(float dt) {
    glUseProgram(advectionShader);
    glUniform1f(glGetUniformLocation(advectionShader, "deltaTime"), dt);
}

void Simulator::recomputeRanges() {
    minParam = glm::vec4(-setting.size, setting.size, 0, setting.kRange.x);
    maxParam = glm::vec4(-setting.size, setting.size, 0, setting.kRange.y);

    unitParam = (maxParam - minParam) / glm::vec4(resolution[0], resolution[1], resolution[2], resolution[3]);
}

void Simulator::recomputeFramebuffer() {
    advectionFBO = std::make_shared<Framebuffer>( resolution[0], resolution[1] );
    diffusionFBO = std::make_shared<Framebuffer>( resolution[0], resolution[1] );
}
