#include "framebuffer.h"
#include "texture.h"
#include "../debug.h"
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

Framebuffer::Framebuffer(int width, int height) : width(width), height(height) {
    glGenFramebuffers(1, &handle);
    bind();
    //glViewport(0, 0, width, height);
    unbind();
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &handle);
    if (rbo) {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::verifyStatus() {
    bind();
    GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Something went wrong: " << err << std::endl;
    }
    unbind();
}

GLuint Framebuffer::getHandle() {
    return handle;
}

void Framebuffer::disableColorDraw() {
    bind();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void Framebuffer::attachTexture(std::shared_ptr<Texture> texture, GLenum attachment, bool is3D) {
    Debug::checkGLError();
    bind();
    Debug::checkGLError();
    if (is3D) glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture->getHandle(), 0);
    else glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->getHandle(), 0);
    Debug::checkGLError();

    if (attachment != GL_DEPTH_ATTACHMENT && attachment != GL_DEPTH_STENCIL_ATTACHMENT) {
        if (std::find(attachments.begin(), attachments.end(), attachment) == attachments.end()) {
            attachments.push_back(attachment);
            Debug::checkGLError();
            glDrawBuffers(attachments.size(), attachments.data());
            Debug::checkGLError();
        }
    }
    Debug::checkGLError();
    unbind();
}

std::shared_ptr<Texture> Framebuffer::createAndAttachColorTexture(GLenum attachment, GLenum texUnit) {
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(texUnit);
    texture->initialize2D(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    attachTexture(texture, attachment);
    Debug::checkGLError();
    return texture;
}

std::shared_ptr<Texture> Framebuffer::createAndAttachVector3Texture(GLenum attachment, GLenum texUnit) {
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(texUnit);
    Debug::checkGLError();
    texture->initialize2D(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    Debug::checkGLError();
    attachTexture(texture, attachment);
    Debug::checkGLError();
    return texture;
}

std::shared_ptr<Texture> Framebuffer::createAndAttachFloatTexture(GLenum attachment, GLenum texUnit) {
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(texUnit);
    texture->initialize2D(width, height, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
    attachTexture(texture, attachment);
    Debug::checkGLError();
    return texture;
}

std::shared_ptr<Texture> Framebuffer::createAndAttachDepthTexture(GLenum texUnit, GLenum interpolationMode, GLenum wrapMode) {
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(texUnit);
    texture->initialize2D(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    texture->setInterpolation(interpolationMode);
    texture->setWrapping(wrapMode);

    attachTexture(texture, GL_DEPTH_ATTACHMENT);
    Debug::checkGLError();
    return texture;
}

std::shared_ptr<Texture> Framebuffer::createAndAttachDepthStencilTexture(GLenum texUnit) {

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(texUnit, GL_TEXTURE_2D);
    texture->initialize2D(width, height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    texture->setInterpolation(GL_LINEAR);
    texture->setWrapping(GL_CLAMP_TO_EDGE);

    attachTexture(texture, GL_DEPTH_STENCIL_ATTACHMENT);
    Debug::checkGLError();
    return texture;
}

void Framebuffer::createAndAttachDepthStencilRenderBuffer() {
    if (rbo) {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }
    glGenRenderbuffers(1, &rbo);
    Debug::checkGLError();
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    Debug::checkGLError();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    Debug::checkGLError();
    bind();
    Debug::checkGLError();
    glad_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    Debug::checkGLError();
    unbind();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Debug::checkGLError();
}
