#include "cubemap.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

CubeMap::CubeMap(std::vector<std::string> filenames, std::vector<GLenum> faces){
    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);

    int width, height, nrChannels;
    for (int i = 0; i<6; i++) {
        unsigned char *data = stbi_load(filenames[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(faces[i], 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            std::cerr<<"Cubemap failed to load (stbi_load)"<<std::endl;
        }
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

CubeMap::~CubeMap(){
    glDeleteTextures(1, &m_handle);
}

void CubeMap::bind(){
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);
}

void CubeMap::unbind(){
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
