#pragma once

#include "cubemap.h"
#include "camera.h"
#include "shaderloader.h"
#include <string>
#include <glad/glad.h>

class Skybox {
public:
    Skybox();
    void draw(glm::mat4 projection, glm::mat4 view);
private:
    //vao vbo  
    GLuint vao, vbo;
    //array of glfloat
    GLfloat pos[108];
    GLuint shaderId; //!< the shader used to render the skybox.
    std::shared_ptr<CubeMap> skyTexture; //!< contains the 6 faces of the sky.
};
