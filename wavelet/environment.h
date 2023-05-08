#pragma once

#include "External/stb/stb_image.h"
#include "GLWrapper/texture.h"
#include "glm/glm.hpp"
#include "glm/vec2.hpp"
#include "wavelet/setting.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <memory>

class Environment {
public:
    /**
     * @brief Construct the environment
     *
     * @param filename = corresponding to the image heightmap which will be used
     * for the environment
     * @param zBoundary = the z value under which we should simulate
     */
    Environment(std::string filename_heightMap, std::string filename_mesh, Setting setting);
    ~Environment();

    bool inDomain(glm::vec2 pos) const;
    // should be distance from boundary
    float levelSet(glm::vec2 pos) const;
    // slope
    glm::vec2 levelSetGradient(glm::vec2 pos) const;

    void draw(glm::mat4 projection, glm::mat4 view);

    void visualize(glm::ivec2 viewport);

    float waterHeight; // where we should simulate water
    // get private set please
    std::shared_ptr<Texture> heightMap, boundaryMap, gradientMap;
private:
    int vaoSize;
    int width, height;
    GLuint shader;
    GLuint visualizationShader;
    GLuint vao, vbo;

    std::vector<float> heights; // we load both in cpu
    std::vector<int> closeToBoundary; // in domain map in case needed
    std::vector<glm::vec2> gradients;

    void getObjData(std::string filename);
};
