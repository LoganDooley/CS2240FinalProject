#pragma once

#include <iostream>
#include <memory>
#include "camera.h"
#include "wavelet/simulator.h"
#include "wavelet/wavegeometry.h"
#include "cubemap.h"
#include "skybox.h"
#include "shaderloader.h"
#include "fullscreenquad.h"

#include <set>

class Core
{
public:
    Core(int width, int height);
    ~Core();
    int update(float seconds);
    int draw();
    void keyEvent(int key, int action);
    void mousePosEvent(double xpos, double ypos);
    void mouseButtonEvent(int button, int action);
    void scrollEvent(double distance);
    void windowResizeEvent(int width, int height);
    void framebufferResizeEvent(int width, int height);

private:
    GLuint m_shader;
    GLuint m_pbShader;
    GLuint m_heightShader;
    std::unique_ptr<WaveGeometry> m_waveGeometry;
    std::shared_ptr<Camera> m_camera;
    std::unique_ptr<Simulator> m_simulator;
    std::shared_ptr<WaveletGrid> m_waveletGrid;
    std::shared_ptr<ProfileBuffer> m_profileBuffer;
    std::shared_ptr<FullscreenQuad> m_fullscreenQuad;
    std::shared_ptr<Environment> m_terrain;
    std::shared_ptr<Skybox> m_skybox;


    glm::ivec2 m_FBOSize = glm::ivec2(640, 480);
    const float FPS = 0.5f;
    float timeSinceLastUpdate;
    bool m_mouseDown = false;
    glm::vec2 m_mousePos = glm::vec2(0, 0);
    std::set<int> m_keysDown = std::set<int>();
    bool simulationPaused = true;
};
