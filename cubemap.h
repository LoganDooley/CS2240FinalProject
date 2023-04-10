#pragma once

#include <glad/glad.h>

#include <vector>
#include <string>

class CubeMap
{
public:
    CubeMap(std::vector<std::string> filenames, std::vector<GLenum> faces);
    ~CubeMap();

    void bind();
    void unbind();

private:
    GLuint m_handle;
};
