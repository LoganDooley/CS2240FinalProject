#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <array>
#include <vector>

struct Setting {
    constexpr static float tau = 6.28318530718f;
    float surfaceTension = 72.8 / 1000; // of water
    float gravity = 9.81;
    float waterViscosity = 1e-5;

    float waterHeight = 0.641;

    float size = 100; // of the simulation square
    glm::vec4 kValues = glm::vec4(0.0045, 0.045, 0.45, 45);

    // resolution x,y,theta,k of the simulator
    std::array<int,4> simulationResolution = {2048, 2048, 8, 4};

    float ambientStrength = 1;

    // random angle i've chosen, feel free to change
    glm::vec2 windDirection = 0.4f * glm::vec2(0, 1);

    // for profile buffer computation
    int integrationNodes = 49;
    int pResolution = 1024;

    float profileBuffer_kMin = 0.1;
    int profileBuffer_kResolution = 4;

    float spatialDiffusionMultiplier = 126.5625;
    float angularDiffusionMultiplier = 0.025;

    // for height field evaluation
    int heightField_resolution = 400;

    float minP() { return 0; }
    float maxP() { return sqrt(2) * size / 2 + 1; }
};
