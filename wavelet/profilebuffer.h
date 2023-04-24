#pragma once

#include <array>
#include <vector>
#include <math.h>

class ProfileBuffer
{
public:
    ProfileBuffer(){};
    ProfileBuffer(float windSpeed);
    ~ProfileBuffer(){};

    void precompute(float t, float k_min, float k_max, int resolution = 4096, int periodicity = 2, int integration_nodes = 100);

    float value(float p) const;

    static float w(float k);

    float psi(float k) const;

    float psiBar(float p, float t, int integration_nodes, float k_min, float k_max);
private:
    float psiBarIntegrand(float k, float p, float t);

    template <typename Function>
    auto integrate(int integration_nodes, float x_min, float x_max, Function const &function){
        float dx = (x_max - x_min) / integration_nodes;
        float x = x_min + 0.5 * dx;

        float result = dx * function(x);
        for(int i = 1; i<integration_nodes; i++) {
            x += dx;
            result += dx * function(x);
        }

        return result;
    }

    float m_windSpeed = 1;
    float m_period = 0;
    std::vector<float> m_data = {};
};
