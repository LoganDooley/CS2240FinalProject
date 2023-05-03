#pragma once

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Texture {
public:
    Texture(GLenum texUnit = GL_TEXTURE0, GLenum texTarget = GL_TEXTURE_2D);

    // load 2D texture from filepath, relative to project's root I believe
    Texture(std::string filePath, GLenum texUnit = GL_TEXTURE0,
            GLint internalFormat = GL_RGBA, GLint format = GL_RGB, GLint dataType = GL_UNSIGNED_BYTE,
            GLenum texTarget = GL_TEXTURE_2D,
            GLenum interpolationMode = GL_LINEAR, GLenum wrapMode = GL_REPEAT);

    ~Texture();

    void initialize3D(int width, int height, int depth,
        GLint internalFormat = GL_RGBA, GLint format = GL_RGBA, GLint dataType = GL_UNSIGNED_BYTE);
    void initialize2D(int width, int height, 
        GLint internalFormat = GL_RGBA, GLint format = GL_RGBA, GLint dataType = GL_UNSIGNED_BYTE);

    void setInterpolation(GLenum interpolationMode);
    void setWrapping(GLenum wrapMode);

    void bind();
    void bind(GLenum texUnit);
    void unbind();
    void unbind(GLenum texUnit);
    void setBorderColor(glm::vec4 color);
    GLuint getHandle();
    GLuint getTexUnitUint();
    GLenum getTexUnitEnum();

private:
    GLuint m_handle;
    GLenum m_texUnit;
    GLenum m_texTarget;
};
