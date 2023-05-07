#include "texture.h"
#include "glm/gtc/type_ptr.hpp"
#include <exception>

#include "stb_image.h"

#include <iostream>

#include "../debug.h"

Texture::Texture(GLenum texUnit, GLenum texTarget)
    : m_texTarget(texTarget), m_texUnit(texUnit) {
    glGenTextures(1, &m_handle);
}

Texture::Texture(std::string filepath, GLenum texUnit, GLint internalFormat,
                 GLint format, GLint dataType, GLenum texTarget,
                 GLenum interpolationMode, GLenum wrapMode)
    : m_texTarget(texTarget), m_texUnit(texUnit) {
    glGenTextures(1, &m_handle);
    bind();
    glTexParameteri(m_texTarget, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(m_texTarget, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(m_texTarget, GL_TEXTURE_MIN_FILTER, interpolationMode);
    glTexParameteri(m_texTarget, GL_TEXTURE_MAG_FILTER, interpolationMode);
    stbi_set_flip_vertically_on_load(1);
    int width, height, numChannels;
    unsigned char *data =
        stbi_load(filepath.c_str(), &width, &height, &numChannels, 4);
    if (stbi_failure_reason()) {
        std::cerr << stbi_failure_reason() << std::endl;
        throw std::runtime_error(stbi_failure_reason());
    }
    glTexImage2D(m_texTarget, 0, internalFormat, width, height, 0, format,
                 dataType, data);
    stbi_image_free(data);
    unbind();
}

Texture::~Texture() {
    glDeleteTextures(1, &m_handle);
}

void Texture::initialize3D(int width, int height, int depth,
                           GLint internalFormat, GLint format, GLint dataType) {

    bind();
    if (width > GL_MAX_TEXTURE_SIZE)
        std::cerr << "Width higher than max size" << std::endl;
    glTexImage3D(m_texTarget, 0, internalFormat, width, height, depth, 0, format,
                 dataType, NULL);
    Debug::checkGLError();
    unbind();
}

void Texture::initialize2D(int width, int height,
                           GLint internalFormat, GLint format, GLint dataType) {
    bind();
    glTexImage2D(m_texTarget, 0, internalFormat, width, height, 0, format,
                 dataType, NULL);
    unbind();
}

void Texture::setInterpolation(GLenum interpolationMode) {
    bind();
    glTexParameteri(m_texTarget, GL_TEXTURE_MIN_FILTER, interpolationMode);
    glTexParameteri(m_texTarget, GL_TEXTURE_MAG_FILTER, interpolationMode);
    Debug::checkGLError();
    unbind();
}

void Texture::setWrapping(GLenum wrapMode) {
    bind();
    glTexParameteri(m_texTarget, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(m_texTarget, GL_TEXTURE_WRAP_T, wrapMode);
    Debug::checkGLError();
    unbind();
}

void Texture::bind() {
    glActiveTexture(m_texUnit);
    glBindTexture(m_texTarget, m_handle);
}

void Texture::bind(GLenum texUnit) {
    glActiveTexture(texUnit);
    glBindTexture(m_texTarget, m_handle);
}

void Texture::unbind() {
    glActiveTexture(m_texUnit);
    glBindTexture(m_texTarget, 0);
}

void Texture::unbind(GLenum texUnit) {
    glActiveTexture(texUnit);
    glBindTexture(m_texTarget, 0);
}

void Texture::setBorderColor(glm::vec4 color) {
    bind();
    glTexParameterfv(m_texTarget, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    unbind();
}

GLuint Texture::getHandle() {
    return m_handle;
}

GLuint Texture::getTexUnitUint() {
    return GLuint(m_texUnit) - GLuint(GL_TEXTURE0);
}

GLenum Texture::getTexUnitEnum() {
    return m_texUnit;
}

void Texture::setValue(glm::ivec2 location, glm::vec4 color){

}
