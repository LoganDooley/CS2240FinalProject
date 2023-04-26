#include "fullscreenquad.h"

FullscreenQuad::FullscreenQuad(){
    GLfloat data[18] = {
        -1, -1, 0,
        1, -1, 0,
        1, 1, 0,
        1, 1, 0,
        -1, 1, 0,
        -1, -1, 0
    };

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), data, GL_DYNAMIC_DRAW);
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

FullscreenQuad::~FullscreenQuad(){

}

void FullscreenQuad::bind(){
    glBindVertexArray(m_vao);
}

void FullscreenQuad::unbind(){
    glBindVertexArray(0);
}
