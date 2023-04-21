#pragma once

#include <iostream>
#include <memory>
#include "camera.h"
#include "wavelet/wavegeometry.h"
#include "shaderloader.h"

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
    std::unique_ptr<WaveGeometry> m_waveGeometry;
    std::unique_ptr<Camera> m_camera;
    std::shared_ptr<WaveletGrid> m_waveletGrid;

    bool m_mouseDown = false;
    glm::vec2 m_mousePos = glm::vec2(0, 0);
    std::set<int> m_keysDown = std::set<int>();
};
