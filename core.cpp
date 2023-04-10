#include "core.h"

#include "debug.h"

Core::Core(int width, int height){
    m_shader = ShaderLoader::createShaderProgram("Shaders/gerstnerWave.vert", "Shaders/gerstnerWave.frag");
    Debug::checkGLError();
    m_camera = std::make_unique<Camera>(width, height, glm::vec3(0, 5, -5), glm::vec3(0, -1, 1), glm::vec3(0, 1, 0), 1.f, 0.1f, 100.f);
    Debug::checkGLError();
    m_water = std::make_unique<Water>(glm::vec2(5, 5), 100);
    Debug::checkGLError();
    m_water->addWave(6, 0.25, glm::vec2(1, 1));
    m_water->addWave(3.1, 0.25, glm::vec2(1, 0.6));
    m_water->addWave(1.8, 0.25, glm::vec2(1, 1.13));
    glDisable(GL_CULL_FACE);
    Debug::checkGLError();
    glViewport(0, 0, width, height);
    Debug::checkGLError();
}

Core::~Core(){

}

int Core::update(float seconds){
    m_camera->move(m_keysDown, seconds);

    return 0;
}

int Core::draw(){
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader);
    m_water->bind();
    m_water->setWaveUniforms(m_shader);
    m_camera->setCameraUniforms(m_shader);
    glUniform1f(glGetUniformLocation(m_shader, "t"), float(glfwGetTime()));
    glDrawArrays(GL_TRIANGLES, 0, m_water->getNumVerts());
    m_water->unbind();
    glUseProgram(0);
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
