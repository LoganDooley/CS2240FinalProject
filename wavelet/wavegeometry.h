#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "waveletgrid.h"
#include <memory>

struct Triangle{
    Triangle(glm::vec3 nv0, glm::vec3 nv1, glm::vec3 nv2):
        v0(nv0), v1(nv1), v2(nv2)
    {

    }

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

struct Tile{
    Tile(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3):
        t1(Triangle(v1, v0, v2)), t2(Triangle(v3, v2, v0))
    {

    }

    Triangle t1;
    Triangle t2;

    std::vector<float> unpack(){
        std::vector<float> data;
        data.resize(3 * 6);

        data[0] = t1.v0.x;
        data[1] = t1.v0.y;
        data[2] = t1.v0.z;

        data[3] = t1.v1.x;
        data[4] = t1.v1.y;
        data[5] = t1.v1.z;

        data[6] = t1.v2.x;
        data[7] = t1.v2.y;
        data[8] = t1.v2.z;

        data[9] = t2.v0.x;
        data[10] = t2.v0.y;
        data[11] = t2.v0.z;

        data[12] = t2.v1.x;
        data[13] = t2.v1.y;
        data[14] = t2.v1.z;

        data[15] = t2.v2.x;
        data[16] = t2.v2.y;
        data[17] = t2.v2.z;

        return data;
    }
};


class WaveGeometry{
public:
    WaveGeometry(glm::vec2 size, unsigned int resolution);
    ~WaveGeometry();

    void bind();
    void unbind();

    void update(std::shared_ptr<WaveletGrid> waveletGrid);

    int getNumVerts();

    void draw(std::shared_ptr<ProfileBuffer> profileBuffer);

private:
    GLuint m_heightShader;

    unsigned int m_resolution;
    glm::vec2 m_size;

    GLuint m_vbo;
    GLuint m_vao;
    int m_numVerts;
};
