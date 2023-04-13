#include "grid.h"
#include <assert.h>
#include <iterator>
#include <math.h>

Grid::Grid() : _resolution({0, 0, 0, 0}), _data() {}

float &Grid::operator()(int xIndex, int yIndex, int kIndex, int thetaIndex) {
    // compile these out of release for efficiency
    assert(xIndex >= 0 && xIndex < _resolution[0]);
    assert(yIndex >= 0 && yIndex < _resolution[1]);
    assert(kIndex >= 0 && kIndex < _resolution[2]);
    assert(thetaIndex >= 0 && thetaIndex < _resolution[3]);

    return _data[xIndex + yIndex * _resolution[0] +
        kIndex * _resolution[1] * _resolution[0] +
        thetaIndex * _resolution[0] * _resolution[1] * _resolution[2]];
}

float const &Grid::operator()(int xIndex, int yIndex, int kIndex,
        int thetaIndex) const {
    // compile these out of release for efficiency
    assert(xIndex >= 0 && xIndex < _resolution[0]);
    assert(yIndex >= 0 && yIndex < _resolution[1]);
    assert(kIndex >= 0 && kIndex < _resolution[2]);
    assert(thetaIndex >= 0 && thetaIndex < _resolution[3]);

    return _data[xIndex + yIndex * _resolution[0] +
        kIndex * _resolution[1] * _resolution[0] +
        thetaIndex * _resolution[0] * _resolution[1] * _resolution[2]];
}

int Grid::getResolution(int dimension) { return _resolution[dimension]; }

void Grid::resize(std::array<int, 4> newResolution) {
    _resolution = newResolution;
    _data.resize(_resolution[0] * _resolution[1] * _resolution[2] *
            _resolution[3]);
}

// wavelet grid

WaveletGrid::WaveletGrid(std::array<int, 4> resolution)
    : _minParam(-size, -size, 0, minZeta), _maxParam(size, size, tau, maxZeta),
    _resolution(resolution) {

        glm::vec4 resolutionVec(resolution[0], resolution[1], resolution[2],
                resolution[3]);
        _pixelParam = (_maxParam - _minParam) / resolutionVec;
    }

float WaveletGrid::angularFrequency(float wavenumber) {
    return sqrt(wavenumber * gravity +
            surfaceTension * wavenumber * wavenumber * wavenumber);
}

float WaveletGrid::advectionSpeed(float wavenumber) {
    float numerator = (gravity + 3 * surfaceTension * wavenumber * wavenumber);
    float denominator = 2 * angularFrequency(wavenumber);
    assert(denominator);
    return numerator / denominator;
}

float WaveletGrid::dispersionSpeed(float wavenumber) {
    // courtesy of wolfram alpha 
    // https://www.wolframalpha.com/input?i=d%5E2%2Fdx%5E2%28sqrt%28ax%2Bbx%5E3%29%29
    float numerator =
        (-2 * gravity + 6 * gravity * surfaceTension * wavenumber * wavenumber +
         3 * surfaceTension * surfaceTension * wavenumber * wavenumber * wavenumber * wavenumber);
    float denom = 4 * pow(wavenumber * (gravity + surfaceTension * wavenumber * wavenumber), 3 / 2);
    assert(denom);
    return numerator / denom;
}

void WaveletGrid::diffusionStep(float deltaTime) {
    float spacialResolution = _pixelParam[X] * _pixelParam[Y];

    for (int i_x = 0; i_x < amplitudes.getResolution(X); i_x++) {
        for (int i_y = 0; i_y < amplitudes.getResolution(Y); i_y++) {
            for (int i_theta = 0; i_theta < amplitudes.getResolution(THETA);
                    i_theta++) {
                for (int i_k = 0; i_k < amplitudes.getResolution(K); i_k++) {
                    glm::vec4 pos = getPositionAtIndex({i_x, i_y, i_theta, i_k});
                    float wavenumber = pos[K];

                    // found on bottom of page 6
                    float delta = 1e-5 * spacialResolution * spacialResolution *
                        (_pixelParam[K] * _pixelParam[K]) * dispersionSpeed(wavenumber);

                    // found on bottom of page 6
                    float gamma = 0.025 * advectionSpeed(wavenumber) * _pixelParam[THETA] *
                        _pixelParam[THETA] / spacialResolution;

                    // TODO: write code to determine if a position is at least 2 away from
                    // boundary
                    bool atLeast2AwayFromBoundary = true;

                    if (atLeast2AwayFromBoundary) {
                    } else {
                    }

                    amplitudes_nxt(i_x, i_y, i_theta, i_k) =
                        amplitudes(i_x, i_y, i_theta, i_k);
                }
            }
        }
    }
    std::swap(amplitudes, amplitudes_nxt);
}

glm::vec4 WaveletGrid::getPositionAtIndex(std::array<int, 4> index) {
    glm::vec4 indexVec(index[0], index[1], index[2], index[3]);
    return _minParam + (indexVec + glm::vec4(0.5)) * _pixelParam;
}
