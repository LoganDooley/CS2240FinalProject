#include "core.h"

#include "debug.h"

Core::Core(int width, int height){
    m_shader = ShaderLoader::createShaderProgram("Shaders/wave.vert", "Shaders/wave.frag");
    Debug::checkGLError();
    m_heightShader = ShaderLoader::createShaderProgram("Shaders/heightEval.vert", "Shaders/heightEval.frag");
    Debug::checkGLError();
    m_camera = std::make_unique<Camera>(width, height, glm::vec3(0, 5, -5), glm::vec3(0, -1, 1), glm::vec3(0, 1, 0), 1.f, 0.1f, 100.f);
    Debug::checkGLError();
    m_waveGeometry = std::make_unique<WaveGeometry>(glm::vec2(50, 50), 100);
    Debug::checkGLError();
    m_profileBuffer = std::make_shared<ProfileBuffer>(1, 4096, 1, 4, 1);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    Debug::checkGLError();
    glViewport(0, 0, width, height);
    Debug::checkGLError();
    m_waveletGrid = std::make_shared<WaveletGrid>(glm::vec4(-50, -50, 0, 1), glm::vec4(50, 50, WaveletGrid::tau, 2), glm::uvec4(100, 100, 16, 4));
    m_waveletGrid->takeStep(0);
    m_waveGeometry->update(m_waveletGrid);
    m_fullscreenQuad = std::make_shared<FullscreenQuad>();
    m_fullscreenQuad->bind();
    m_profileBuffer->precomputeGPU(0, 2, 100);
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
    m_profileBuffer->precomputeGPU(glfwGetTime(), 2, 100);
    m_waveGeometry->draw(m_profileBuffer);
    return 0;
}

int Core::draw(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /*
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader);
    m_waveGeometry->bind();
    m_camera->setCameraUniforms(m_shader);
    glDrawArrays(GL_TRIANGLES, 0, m_waveGeometry->getNumVerts());
    Debug::checkGLError();
    m_waveGeometry->unbind();
    glUseProgram(0);
    */
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
}

void Core::windowResizeEvent(int width, int height){
    m_camera->resize(width, height);
}
