#pragma once

#include <array>
#include <vector>
#include <math.h>

class ProfileBuffer
{
public:
    ProfileBuffer(float windSpeed);
    ~ProfileBuffer();

    void precompute(float t, float k_min, float k_max, int resolution, int periodicity, int integration_nodes);

private:
    float psi(float k) const;

    float w(float k) const;

    float psiBarIntegrand(float k, float p, float t);
    float psiBar(float p, float t, int integration_nodes, float k_min, float k_max);

    template <typename Function>
    auto integrate(int integration_nodes, double x_min, double x_max, Function const &function){
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
    float m_period;
    std::vector<float> m_data;
};
