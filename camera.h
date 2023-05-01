#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <set>

class Camera{
public:
    Camera(int width, int height, glm::vec3 pos, glm::vec3 look, glm::vec3 up, float fov, float near, float far);
    ~Camera();

    glm::mat4 getProjection();
    glm::mat4 getView();

    void resize(int width, int height);

    glm::vec3 getLook();
    glm::vec3 getPos();

    void setCameraUniforms(GLuint shader);
    
    void move(std::set<int>& keysDown, float dt);
    void rotate(glm::vec2 deltaMousePos);

    int getWidth() const{
        return m_width;
    }

    int getHeight() const{
        return m_height;
    }

private:
    void translate(glm::vec3 move);
    void angleAxis(float angle, glm::vec3 axis);

    void calculateProjection();
    void calculateView();

    int m_width;
    int m_height;
    glm::vec3 m_pos;
    glm::vec3 m_look;
    glm::vec3 m_up;
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    glm::mat4 m_proj = glm::mat4(1);
    glm::mat4 m_view = glm::mat4(1);
};
