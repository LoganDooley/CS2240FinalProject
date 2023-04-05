#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glad/glad.h>

class Camera{
public:
    Camera(int width, int height, glm::vec3 pos, glm::vec3 look, glm::vec3 up, float fov, float near, float far);
    ~Camera();

    glm::mat4 getProjection();
    glm::mat4 getView();

    void resize(int width, int height);
    void translate(glm::vec3 move);
    void rotate(float angle, glm::vec3 axis);

    glm::vec3 getLook();
    glm::vec3 getPos();

    void setCameraUniforms(GLuint shader);
    
private:
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