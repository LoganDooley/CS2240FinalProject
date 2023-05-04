#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <array>
#include <vector>

struct Setting {
    float surfaceTension = 72.8 / 1000; // of water
    float gravity = 9.81;

    float size = 100; // of the simulation square
    glm::vec4 kValues = glm::vec4(0.0045, 0.045, 0.45, 45);

    // resolution x,y,theta,k of the simulator
    std::array<int,4> simulationResolution = {2048, 2048, 8, 4};

    // random angle i've chosen, feel free to change
    glm::vec2 windDirection = 0.4f * glm::vec2(std::cos(1), std::sin(1));

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
