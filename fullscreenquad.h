#pragma once

#include <glad/glad.h>

#include <vector>
#include <string>

class FullscreenQuad
{
public:
    FullscreenQuad();
    ~FullscreenQuad();

    void bind();
    void unbind();

private:
    GLuint m_vbo;
    GLuint m_vao;
};
