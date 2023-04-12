#include "grid.h"
#include <math.h>
#include <assert.h>

float &Grid::operator()(int xIndex, int yIndex, int kIndex, int thetaIndex) {
    // compile these out of release for efficiency
    assert(xIndex >= 0 && xIndex < _resolution[0]);
    assert(yIndex >= 0 && yIndex < _resolution[1]);
    assert(kIndex >= 0 && kIndex < _resolution[2]);
    assert(thetaIndex >= 0 && thetaIndex < _resolution[3]);

    return _data[xIndex + yIndex * _resolution[0] + kIndex * _resolution[1] * _resolution[0] + 
        thetaIndex * _resolution[0] * _resolution[1] * _resolution[2]];
}

float const &Grid::operator()(int xIndex, int yIndex, int kIndex, int thetaIndex) const {
    // compile these out of release for efficiency
    assert(xIndex >= 0 && xIndex < _resolution[0]);
    assert(yIndex >= 0 && yIndex < _resolution[1]);
    assert(kIndex >= 0 && kIndex < _resolution[2]);
    assert(thetaIndex >= 0 && thetaIndex < _resolution[3]);

    return _data[xIndex + yIndex * _resolution[0] + kIndex * _resolution[1] * _resolution[0] + 
        thetaIndex * _resolution[0] * _resolution[1] * _resolution[2]];
}

float WaveletGrid::angularFrequency(float wavenumber) {
    return sqrt(wavenumber * gravity + surfaceTension * wavenumber * wavenumber * wavenumber);
}

float WaveletGrid::groupSpeed(float wavenumber) {
    float numerator = (gravity + 3 * surfaceTension * wavenumber * wavenumber);
    float denominator = 2 * angularFrequency(wavenumber);
    assert(denominator);
    return numerator / denominator;
}
