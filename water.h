#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

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
        t1(Triangle(v0, v1, v2)), t2(Triangle(v2, v3, v0))
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

struct Wave{
    Wave(float wavelength, float steepness, glm::vec2 direction){
        m_wavelength = wavelength;
        m_wavenumber = 2 * M_PI / m_wavelength;
        m_steepness = fmaxf(fminf(steepness, 1.f), 0.f);
        m_amplitude = m_steepness/m_wavenumber;
        m_direction = glm::normalize(direction);
        m_dispersion = sqrtf(9.8f * m_wavenumber);
    }
    float m_wavelength;
    float m_steepness;
    float m_dispersion;
    float m_wavenumber;
    float m_amplitude;
    glm::vec2 m_direction;
};

class Water{
public:
    Water(glm::vec2 size, unsigned int resolution);
    ~Water();

    void bind();
    void unbind();

    int getNumVerts();

    void addWave(float wavelength, float steepness, glm::vec2 direction);
    void setWaveUniforms(GLuint shader);

private:

    GLuint m_vbo;
    GLuint m_vao;
    int m_numVerts;
    std::vector<Wave> m_waves;
};