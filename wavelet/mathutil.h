#pragma once

#include <functional>

namespace Math {
    float interpolate(float t, std::function<float(int)> f);
    float interpolate2D(float x, float y, std::function<float(int, int)> f);
}
