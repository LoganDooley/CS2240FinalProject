#include "environment.h"
#include "GLWrapper/texture.h"
#include "debug.h"
#include "glm/gtc/type_ptr.hpp"
#include "shaderloader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <cfloat>

Environment::Environment(std::string heightMapFilename, std::string meshFileName, float zBoundary) : zBoundary(zBoundary) {
    int n;
    unsigned char *data = stbi_load(heightMapFilename.c_str(), &width, &height, &n, 0);
    if (data == NULL) {
        std::cout << "Error loading image:" << stbi_failure_reason() << std::endl;
        exit(1);
    }
    heights.resize(width * height);
    gradients.resize(width * height);
    closeToBoundary.resize(width * height);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int index = (i + j * width) * n;
            float r = data[index] / 255.0f;
            float g = data[index + 1] / 255.0f;
            float b = data[index + 2] / 255.0f;
            float height = (r + g + b) / 3.0f;
            heights[i * width + j] = height;
            gradients[i * width + j] = glm::vec2(0, 0);
        }
    }
    auto sample = [&](int i, int j) {
        i = std::clamp(i, 0, width-1);
        j = std::clamp(j, 0, height-1);
        return heights[i * width + j];
    };
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float dx = (sample(i + 1, j) - sample(i - 1, j)) / 2;
            float dy = (sample(i, j+1) - sample(i, j-1)) / 2;

            gradients[i * width + j] = glm::normalize(glm::vec2(dx, dy));
            closeToBoundary[i * width + j] = 0;
            for (int dx = -2; dx <= 2; dx++)
                for (int dy = -2; dy <= 2; dy++)
                    closeToBoundary[i * width + j] |= sample(i + dx, j + dy) < zBoundary;
        }
    }
    stbi_image_free(data);
    this->zBoundary = zBoundary;

    // textures initialization and data loading
    {
        std::cout << ("initialize height map") << std::endl;
        heightMap = std::make_shared<Texture>();
        heightMap->setInterpolation(GL_LINEAR);
        heightMap->setWrapping(GL_CLAMP_TO_EDGE);
        heightMap->initialize2D(width, height,
            GL_R16F, GL_RED, GL_FLOAT, heights.data());
        Debug::checkGLError();

        std::cout << ("initialize boundary map") << std::endl;
        boundaryMap = std::make_shared<Texture>();
        boundaryMap->setInterpolation(GL_LINEAR);
        boundaryMap->setWrapping(GL_CLAMP_TO_EDGE);
        boundaryMap->initialize2D(width, height,
            GL_R8, GL_RED, GL_INT, closeToBoundary.data());
        Debug::checkGLError();

        std::cout << ("initialize gradient map") << std::endl;

        std::vector<float> data(gradients.size() * 2);
        for (int i = 0; i < gradients.size(); i++) {
            data[i<<1|0] = gradients[i].r;
            data[i<<1|1] = gradients[i].g;
        }
        gradientMap = std::make_shared<Texture>();
        gradientMap->setInterpolation(GL_LINEAR);
        gradientMap->setWrapping(GL_CLAMP_TO_EDGE);
        gradientMap->initialize2D(width, height,
            GL_RG16F, GL_RG, GL_FLOAT, data.data());
        Debug::checkGLError();
    }

    // loading geometry mesh
    getObjData(meshFileName);

    shader = ShaderLoader::createShaderProgram("Shaders/terrain.vert", "Shaders/terrain.frag");
}

Environment::~Environment() {
    if (shader) glDeleteProgram(shader);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
}

void Environment::draw(glm::mat4 projection, glm::mat4 view) {
    glm::mat4 model = glm::mat4(1); // identity lol
    glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vaoSize);
    glBindVertexArray(0);
    glUseProgram(0);
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

void Environment::getObjData(std::string filepath){
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
    for(int i = 0; i<faces.size(); i++){
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

