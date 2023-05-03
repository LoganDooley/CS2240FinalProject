#pragma once

#include <glm/glm.hpp>
#include <array>

struct Setting {
    float surfaceTension = 72.8 / 1000; // of water
    float gravity = 9.81;

    float size = 100; // of the simulation square
    glm::vec2 kRange = glm::vec2(0.01, 10); // linear in this range

    // resolution x,y,theta,k of the simulator
    std::array<int,4> simulationResolution = {4096, 4096, 16, 4};

    // for profile buffer computation
    int integrationNodes = 49;
    int pResolution = 1024;

    float profileBuffer_kMin = 0.1;
    int profileBuffer_kResolution = 4;

    // for height field evaluation
    int heightField_resolution = 400;

    float minP() { return 0; }
    float maxP() { return sqrt(2) * size / 2 + 1; }
};
