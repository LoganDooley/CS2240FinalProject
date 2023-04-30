#include "core.h"

#include "debug.h"
#include "wavelet/simulator.h"

Core::Core(int width, int height){
    m_shader = ShaderLoader::createShaderProgram("Shaders/wave.vert", "Shaders/wave.frag");
    Debug::checkGLError();

    m_heightShader = ShaderLoader::createShaderProgram("Shaders/heightEval.vert", "Shaders/heightEval.frag");
    Debug::checkGLError();

    m_camera = std::make_unique<Camera>(width, height, glm::vec3(0, 5, -5), glm::vec3(0, -1, 1), glm::vec3(0, 1, 0), 1.f, 0.1f, 100.f);
    Debug::checkGLError();

    m_waveGeometry = std::make_unique<WaveGeometry>(glm::vec2(10, 10), 400);
    Debug::checkGLError();

    std::array<int,4> resolution = {4096, 4096, 16, 4};
    Simulator::GridSettings setting;
    m_simulator = std::make_unique<Simulator>(resolution, setting);
    Debug::checkGLError();


    m_profileBuffer = std::make_shared<ProfileBuffer>(1, 4096, 0.01, 50, 4);
    glEnable(GL_CULL_FACE);


    glEnable(GL_DEPTH_TEST);
    Debug::checkGLError();
    glViewport(0, 0, width, height);
    Debug::checkGLError();
    m_waveletGrid = std::make_shared<WaveletGrid>(glm::vec4(-50, -50, 0, 1), glm::vec4(50, 50, WaveletGrid::tau, 2), glm::uvec4(100, 100, 16, 4));
    //m_waveletGrid->takeStep(0);
    //m_waveGeometry->update(m_waveletGrid);
    m_fullscreenQuad = std::make_shared<FullscreenQuad>();
    m_fullscreenQuad->bind();
}

Core::~Core(){

}

int Core::update(float seconds){
    /*
    m_camera->move(m_keysDown, seconds);
    if (timeSinceLastUpdate += seconds >= 1.0f/FPS) {
        m_waveletGrid->takeStep(seconds);
        m_waveGeometry->update(m_waveletGrid);
        timeSinceLastUpdate = 0;
    }
    */
    m_simulator->takeStep(seconds);
    m_profileBuffer->precomputeGPU(glfwGetTime());
    glViewport(0, 0, m_FBOSize.x, m_FBOSize.y);
    //m_profileBuffer->debugDraw();
    m_waveGeometry->draw(m_profileBuffer);
    return 0;
}

int Core::draw(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    return 0;
}

void Core::keyEvent(int key, int action){
    if(action == GLFW_PRESS){
        m_keysDown.insert(key);
    }
    else if(action == GLFW_RELEASE){
        m_keysDown.erase(key);
    }
}

void Core::mousePosEvent(double xpos, double ypos){
    if(m_mouseDown){
        m_camera->rotate(glm::vec2(xpos, ypos) - m_mousePos);
    }
    m_mousePos = glm::vec2(xpos, ypos);
}

void Core::mouseButtonEvent(int button, int action){
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        m_mouseDown = true;
    }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
        m_mouseDown = false;
    }
}

void Core::scrollEvent(double distance){
    
}

void Core::framebufferResizeEvent(int width, int height){
    glViewport(0, 0, width, height);
    m_FBOSize = glm::ivec2(width, height);
}

void Core::windowResizeEvent(int width, int height){
    m_camera->resize(width, height);
}
