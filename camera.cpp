#include "camera.h"
#include <iostream>

Camera::Camera(int width, int height, glm::vec3 pos, glm::vec3 look, 
    glm::vec3 up, float fov, float nearPlane, float farPlane):
    m_width(width),
    m_height(height),
    m_pos(pos),
    m_look(look),
    m_up(up),
    m_fov(fov),
    m_aspect(width/float(height)),
    m_near(nearPlane),
    m_far(farPlane)
{
    calculateProjection();
    calculateView();
}

Camera::~Camera(){
    
}

glm::mat4 Camera::getProjection(){
    return m_proj;
}

glm::mat4 Camera::getView(){
    return m_view;
}

glm::vec3 Camera::getLook(){
    return m_look;
}

glm::vec3 Camera::getPos(){
    return m_pos;
}

void Camera::setCameraUniforms(GLuint shader){
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(m_proj));
}

void Camera::resize(int width, int height){
    m_width = width;
    m_height = height;
    m_aspect = float(width)/float(height);

    calculateProjection();
}

void Camera::translate(glm::vec3 move){
    m_pos += move;

    calculateView();
}

void Camera::angleAxis(float angle, glm::vec3 axis){
    glm::mat4 lookRotation = glm::rotate(glm::mat4(1), angle, axis);
    glm::vec3 tempLook = glm::vec3(lookRotation * glm::vec4(m_look, 0));
    if(glm::cross(tempLook, m_up) != glm::vec3(0)){
        m_look = tempLook;
       calculateView();
    }
}

void Camera::calculateProjection(){
    m_proj = glm::perspective(m_fov, m_aspect, m_near, m_far);
}

void Camera::calculateView(){
    m_view = glm::lookAt(m_pos, m_pos+m_look, m_up);
}

void Camera::move(std::set<int>& keysDown, float dt){
    glm::vec3 moveDir = glm::vec3(0);
    glm::vec3 front = glm::vec3(m_look.x, 0, m_look.z);
    glm::vec3 side = glm::vec3(front.z, 0, -front.x);
    glm::vec3 up = glm::vec3(0, 1, 0);
    if(keysDown.count(GLFW_KEY_W) != 0){
        moveDir += front;
    }
    if(keysDown.count(GLFW_KEY_S) != 0){
        moveDir -= front;
    }
    if(keysDown.count(GLFW_KEY_A) != 0){
        moveDir += side;
    }
    if(keysDown.count(GLFW_KEY_D) != 0){
        moveDir -= side;
    }
    if(keysDown.count(GLFW_KEY_SPACE) != 0){
        moveDir += up;
    }
    if(keysDown.count(GLFW_KEY_LEFT_SHIFT) != 0){
        moveDir -= up;
    }

    if(moveDir != glm::vec3(0)){
        moveDir = glm::normalize(moveDir);
        translate(moveDir * dt);
    }
}

void Camera::rotate(glm::vec2 deltaMousePos){
    float sensitivity = 0.025f;
    glm::vec3 xAxis = glm::vec3(0, 1, 0);
    glm::vec3 yAxis = glm::vec3(m_look.z, 0, -m_look.x);

    angleAxis(-deltaMousePos.x * sensitivity, xAxis);
    angleAxis(deltaMousePos.y * sensitivity, yAxis);
}
