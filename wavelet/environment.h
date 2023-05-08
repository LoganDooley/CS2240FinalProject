#pragma once

#include "External/stb/stb_image.h"
#include "GLWrapper/texture.h"
#include "glm/glm.hpp"
#include "glm/vec2.hpp"
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
    Environment(std::string filename_heightMap, std::string filename_mesh, float zBoundary);
    ~Environment();

    bool inDomain(glm::vec2 pos) const;
    // should be distance from boundary
    float levelSet(glm::vec2 pos) const;
    // slope
    glm::vec2 levelSetGradient(glm::vec2 pos) const;

    void draw(glm::mat4 projection, glm::mat4 view);

private:
    int vaoSize;
    int width, height;
    float zBoundary; // where we should simulate water
    GLuint shader;
    GLuint vao, vbo;
    std::shared_ptr<Texture> heightMap, boundaryMap, gradientMap;

    std::vector<float> heights; // we load both in cpu
    std::vector<int> closeToBoundary; // in domain map in case needed
    std::vector<glm::vec2> gradients;

    void getObjData(std::string filename);
};
