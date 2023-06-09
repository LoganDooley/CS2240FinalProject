#include "environment.h"
#include "GLWrapper/texture.h"
#include "debug.h"
#include "glm/gtc/type_ptr.hpp"
#include "shaderloader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <cfloat>
#include <queue>
#include <imgui.h>

Environment::Environment(std::string heightMapFilename, std::string meshFileName, Setting setting)
    : waterHeight(setting.waterHeight) {
    int n;
    unsigned char *data = stbi_load(heightMapFilename.c_str(), &width, &height, &n, 0);
    if (data == NULL) {
        std::cout << "Error loading image:" << stbi_failure_reason() << std::endl;
        exit(1);
    }
    heights.resize(width * height);
    gradients.resize(width * height);
    gradientTheta.resize(width * height);
    closeToBoundary.resize(width * height);
    closestOnBoundary.resize(width * height);
    heightWithSampleLocationInDomain.resize(width * height);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int index = (i + j * width) * n;
            float r = data[index] / 255.0f;
            float g = data[index + 1] / 255.0f;
            float b = data[index + 2] / 255.0f;
            float heightVal = (r + g + b) / 3.0f;
            heights[(width - 1 - i) + width * (height - 1 - j)] = heightVal;
            gradients[(width - 1 - i) + width * (height - 1 - j)] = glm::vec2(0, 0);
        }
    }
    auto sample = [&](int i, int j) {
        i = std::clamp(i, 0, width-1);
        j = std::clamp(j, 0, height-1);
        return heights[i + j * width];
    };
    glm::vec2 unitDistance(setting.size / width, setting.size / height);
    glm::vec2 inverseTwiceUnitDistance(2.0f/unitDistance.x, 2.0f/unitDistance.y);

    float kernel[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1},
    };

    /* float kernel[3][3] = { */
    /*     {1, 0, 0}, */
    /*     {0, 1, 0}, */
    /*     {0, 0, 1} */
    /* }; */

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int index = i + j * width;

            long double gx = 0, gy = 0;

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    gy += kernel[dy+1][dx+1] * sample(dx + i, dy + j);
                    gx += kernel[dx+1][dy+1] * sample(dx + i, dy + j);
                }
            }

            /* glm::vec2 grad = glm::normalize(glm::vec2(gx, gy)); */
            /* glm::vec2 grad = glm::normalize(glm::vec2(0,1)); */
            glm::vec2 grad = glm::vec2(gx, gy);
            gradients[index] = grad;
            if (gx || gy)       gradientTheta[index] = std::atan2(gy, gx);
            else                gradientTheta[index] = 0;
            gradientTheta[index] /= setting.tau;
            if (gradientTheta[index] < 0) gradientTheta[index]++;

            closeToBoundary[index] = 0;
            int range = 1;
            for (int dx = -range; dx <= range; dx++)
                for (int dy = -range; dy <= range; dy++) {
                    float close = sample(i + dx, j + dy) > waterHeight;
                    closeToBoundary[index] = std::max(close, closeToBoundary[index]);
                }
        }
    }

    { // compute closest to boundary
        std::fill(closestOnBoundary.begin(), closestOnBoundary.end(), glm::ivec2(-1,-1));
        // we shall bfs
        std::queue<glm::ivec2> candidatePositions;

        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                if (sample(i,j) <= waterHeight) {
                    candidatePositions.push(glm::ivec2(i,j));
                    closestOnBoundary[i + j*width] = glm::ivec2(i,j);
                }
            }
        }

        int di[] = {-1, 1, 0, 0};
        int dj[] = {0, 0, -1, 1};

        while (candidatePositions.size()) {
            glm::ivec2 canPosition = candidatePositions.front();
            candidatePositions.pop();
            int i = canPosition.x,  j = canPosition.y;

            for (int k = 0; k < 4; k++) {
                int ni = canPosition.x + di[k], nj = canPosition.y + dj[k];
                if (ni >= 0 && ni < width && nj >= 0 && nj < height 
                        && closestOnBoundary[ni + nj * width] == glm::ivec2(-1,-1)) {

                    closestOnBoundary[ni + nj * width] = closestOnBoundary[i + j * width];
                    candidatePositions.push(glm::ivec2(ni,nj));
                }
            }
        }

        for (int i = 0; i < width * height; i++) {
            heightWithSampleLocationInDomain[i] = glm::vec3(
                heights[i],
                (closestOnBoundary[i].x + 0.5f) / width,
                (closestOnBoundary[i].y + 0.5f) / height
            );
        }
    }

    stbi_image_free(data);

    // textures initialization and data loading
    {
        std::cout << ("initialize height map") << std::endl;
        heightMap = std::make_shared<Texture>();
        heightMap->setInterpolation(GL_NEAREST);
        heightMap->setWrapping(GL_CLAMP_TO_EDGE);
        heightMap->initialize2D(width, height,
                                GL_RGB16F, GL_RGB, GL_FLOAT, heightWithSampleLocationInDomain.data()); // TODO: MUST BE FIXED
        Debug::checkGLError();

        std::cout << ("initialize boundary map") << std::endl;
        boundaryMap = std::make_shared<Texture>();
        boundaryMap->setInterpolation(GL_NEAREST);
        boundaryMap->setWrapping(GL_CLAMP_TO_EDGE);
        boundaryMap->initialize2D(width, height,
                                  GL_R8, GL_RED, GL_FLOAT, closeToBoundary.data());
        Debug::checkGLError();

        std::cout << ("initialize gradient map") << std::endl;

        std::vector<float> data(gradients.size() * 2);
        for (int i = 0; i < gradients.size(); i++) {
            data[i<<1|0] = gradients[i].r;
            data[i<<1|1] = gradients[i].g;
        }
        gradientMap = std::make_shared<Texture>();
        gradientMap->setInterpolation(GL_NEAREST);
        gradientMap->setWrapping(GL_CLAMP_TO_EDGE);
        gradientMap->initialize2D(width, height,
                                  GL_RG32F, GL_RG, GL_FLOAT, data.data());
        Debug::checkGLError();
    }

    // loading geometry mesh
    getObjData(meshFileName);

    terrainShader = ShaderLoader::createShaderProgram("Shaders/terrain.vert", "Shaders/terrain.frag");
    Debug::checkGLError();

    visualizationShader = ShaderLoader::createShaderProgram(
                              "Shaders/texture.vert",
                              "Shaders/environment_visualization.frag"
                          );
    glUseProgram(visualizationShader);
    glUniform1i(glGetUniformLocation(visualizationShader, "heightMap"), 0);
    glUniform1i(glGetUniformLocation(visualizationShader, "boundaryMap"), 1);
    glUniform1i(glGetUniformLocation(visualizationShader, "gradientMap"), 2);
    Debug::checkGLError();
}

Environment::~Environment() {
    if (terrainShader) glDeleteProgram(terrainShader);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (visualizationShader) glDeleteProgram(visualizationShader);
}

void Environment::draw(glm::mat4 projection, glm::mat4 view) {

    glm::mat4 model = glm::mat4(10); // identity lol
    glUseProgram(terrainShader);
    glUniformMatrix4fv(glGetUniformLocation(terrainShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(terrainShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(terrainShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vaoSize);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Environment::visualize(glm::ivec2 viewport) {
    ImGui::SliderInt("which visualization", &toVisualize, 0, 5);
    glUseProgram(visualizationShader);
    glUniform1i(glGetUniformLocation(visualizationShader, "whichVisualization"), toVisualize);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    heightMap->bind(GL_TEXTURE0);
    boundaryMap->bind(GL_TEXTURE1);
    gradientMap->bind(GL_TEXTURE2);

    int n = std::min(viewport.x, viewport.y);
    glViewport(0, 0, n, n);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);

    heightMap->unbind(GL_TEXTURE0);
    boundaryMap->unbind(GL_TEXTURE1);
    gradientMap->unbind(GL_TEXTURE2);
}

bool Environment::inDomain(glm::vec2 pos) const {
    return levelSet(pos) >= 0;
}

float Environment::levelSet(glm::vec2 pos) const {
    /* if (pos.x < 0 || pos.x >= widt || pos.y < 0 || pos.y >= heights[0].size()) */
    /*     return -1; */
    return -1;
    /* float zVal = heights[(int)((pos.x))][(int)((pos.y))]; */
    /* if (zVal < zBoundary) { */
    /*     return zBoundary - zVal; */
    /* } else { */
    /*     return -1; */
    /* } */
}

glm::vec2 Environment::levelSetGradient(glm::vec2 pos) const {
    return glm::vec2(0,0);
    /* return gradients[(int)((pos.x))][(int)((pos.y))]; */
}

void Environment::getObjData(std::string filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::vector<std::array<tinyobj::index_t, 3>> faces;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    for(size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            unsigned int fv = shapes[s].mesh.num_face_vertices[f];

            std::array<tinyobj::index_t, 3> face;
            for(size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                face[v] = idx;

            }
            faces.push_back(face);

            index_offset += fv;
        }
    }
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        vertices.emplace_back(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
    }
    for (size_t i = 0; i < attrib.normals.size(); i += 3) {
        normals.emplace_back(attrib.normals[i], attrib.normals[i+1], attrib.normals[i+2]);
    }
    std::vector<float> data;
    data.resize(18 * faces.size());
    for(int i = 0; i<faces.size(); i++) {
        glm::vec3 v0 = vertices[static_cast<int>(faces[i][0].vertex_index)];
        glm::vec3 v1 = vertices[static_cast<int>(faces[i][1].vertex_index)];
        glm::vec3 v2 = vertices[static_cast<int>(faces[i][2].vertex_index)];
        glm::vec3 triangleNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        glm::vec3 normal0 = normals.size() ? normals[static_cast<int>(faces[i][0].normal_index)] : triangleNormal;
        glm::vec3 normal1 = normals.size() ? normals[static_cast<int>(faces[i][1].normal_index)] : triangleNormal;
        glm::vec3 normal2 = normals.size() ? normals[static_cast<int>(faces[i][2].normal_index)] : triangleNormal;

        data[18*i] = v0.x;
        data[18*i+1] = v0.y;
        data[18*i+2] = v0.z;
        data[18*i+3] = normal0.x;
        data[18*i+4] = normal0.y;
        data[18*i+5] = normal0.z;
        data[18*i+6] = v1.x;
        data[18*i+7] = v1.y;
        data[18*i+8] = v1.z;
        data[18*i+9] = normal1.x;
        data[18*i+10] = normal1.y;
        data[18*i+11] = normal1.z;
        data[18*i+12] = v2.x;
        data[18*i+13] = v2.y;
        data[18*i+14] = v2.z;
        data[18*i+15] = normal2.x;
        data[18*i+16] = normal2.y;
        data[18*i+17] = normal2.z;
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STATIC_DRAW);
    Debug::checkGLError();

    // positions
    glEnableVertexAttribArray(0);
    Debug::checkGLError();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat),
                          reinterpret_cast<void*>(0*sizeof(GLfloat)));
    Debug::checkGLError();

    // normals
    glEnableVertexAttribArray(1);
    Debug::checkGLError();
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat),
                          reinterpret_cast<void*>(3*sizeof(GLfloat)));
    Debug::checkGLError();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Debug::checkGLError();

    vaoSize = data.size() / 6;
}
