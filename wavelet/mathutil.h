#pragma once

#include <functional>

namespace Math {
    float interpolate(float t, std::function<float(float)> f);
}

