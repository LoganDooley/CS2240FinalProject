#pragma once

#include <functional>

namespace Math {
    /**
     * Interpolate a function defined on integer coordiantes using cubic interpolation.
     *
     * @param t the coordinate of the function to evaluate.
     * @param f the function to evaluate.
     * @return interpolated f(t).
     */
    float interpolate(float t, std::function<float(int)> f);
    /**
     * Interpolate a function defined on integer coordiantes using cubic interpolation.
     *
     * @param t the coordinate of the function to evaluate.
     * @param f the function to evaluate.
     * @return interpolated f(t).
     */
    float interpolate2D(float x, float y, std::function<float(int, int)> f);
}
