#pragma once

#include <array>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

enum Parameter{
    X = 0,
    Y = 1,
    THETA = 2,
    K = 3
};


class Amplitude
{
public:
    Amplitude(){};
    ~Amplitude(){};

    unsigned int getResolution(Parameter p);
    void resize(glm::uvec4 newResolution);

    float &operator()(glm::uvec4 index);
    float const &operator()(glm::uvec4 index) const;

    void setTemporaryData();

private:
    unsigned int dataIndex(glm::uvec4 index) const;

    std::vector<float> m_data = {};
    glm::uvec4 m_resolution = glm::uvec4(0, 0, 0, 0);
};
