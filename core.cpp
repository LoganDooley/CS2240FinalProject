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
    glm::vec3 moveDir = glm::vec3(0);
    glm::vec3 front = glm::vec3(m_camera->getLook().x, 0, m_camera->getLook().z);
    glm::vec3 side = glm::vec3(front.z, 0, -front.x);
    glm::vec3 up = glm::vec3(0, 1, 0);
    if(m_keysDown.count(GLFW_KEY_W) != 0){
        moveDir += front;
    }
    if(m_keysDown.count(GLFW_KEY_S) != 0){
        moveDir -= front;
    }
    if(m_keysDown.count(GLFW_KEY_A) != 0){
        moveDir += side;
    }
    if(m_keysDown.count(GLFW_KEY_D) != 0){
        moveDir -= side;
    }
    if(m_keysDown.count(GLFW_KEY_SPACE) != 0){
        moveDir += up;
    }
    if(m_keysDown.count(GLFW_KEY_LEFT_SHIFT) != 0){
        moveDir -= up;
    }

    if(moveDir != glm::vec3(0)){
        moveDir = glm::normalize(moveDir);
        m_camera->translate(moveDir * seconds);
    }

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

}

void Core::mouseButtonEvent(int button, int action){

}

void Core::scrollEvent(double distance){
    
}

void Core::framebufferResizeEvent(int width, int height){
    glViewport(0, 0, width, height);
}

void Core::windowResizeEvent(int width, int height){

}
