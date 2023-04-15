#pragma once

#include <array>
#include <vector>

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
    void resize(std::array<unsigned int, 4> newResolution);

    float &operator()(unsigned int xIndex, unsigned int yIndex, unsigned int thetaIndex, unsigned int kIndex);
    float const &operator()(unsigned int xIndex, unsigned int yIndex, unsigned int thetaIndex, unsigned int kIndex) const;

    void setTemporaryData();

private:
    unsigned int dataIndex(unsigned int xIndex, unsigned int yIndex, unsigned int thetaIndex, unsigned int kIndex) const;

    std::vector<float> m_data = {};
    std::array<unsigned int, 4> m_resolution = {0, 0, 0, 0};
};
