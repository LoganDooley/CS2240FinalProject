#pragma once

#include "wavelet/environment.h"
#include <functional>

namespace Math {
    /**
     * @brief Interpolate a function defined on integer coordiantes using cubic interpolation.
     *
     * @param t the coordinate of the function to evaluate.
     * @param f the function to evaluate.
     * @return interpolated f(t).
     */
    float interpolate(float t, std::function<float(int)> f);
    /**
     * @brief Interpolate a function defined on integer coordiantes using cubic interpolation.
     *
     * @param t the coordinate of the function to evaluate.
     * @param f the function to evaluate.
     * @return interpolated f(t).
     */
    float interpolate2D(float x, float y, std::function<float(int, int)> f);

    /**
     * @brief Interpolate a function defined over a 4D integer coordinate grid.
     *
     * @param x the x coordinate.
     * @param y the y coordinate.
     * @param theta the theta coordinate.
     * @param wavenumber the wavenumber.
     * @param f the function on integer coordinates
     * @param domain the domain function that tells if something is inside the domain. f values outside of the domain
     * are undefined, and is not considered in the interpolation.
     */
    float interpolate4D(float x, float y, float theta, float wavenumber, std::function<float(int,int,int,int)> f, Environment environment);
}
