#pragma once

#include "External/stb/stb_image.h"
#include "glm/glm.hpp"
#include "glm/vec2.hpp"
#include <iostream>
#include <vector>

class Environment {
public:
  Environment() = default;

  /**
   * @brief Construct the environment
   *
   * @param filename = corresponding to the image heightmap which will be used
   * for the environment
   * @param zBoundary = the z value under which we should simulate
   */
  Environment(std::string filename, float zBoundary);
  bool inDomain(glm::vec2 pos) const;
  // should be distance from boundary
  float levelSet(glm::vec2 pos) const;
  // slope
  glm::vec2 levelSetGradient(glm::vec2 pos) const;

private:
  float zBoundary;
  std::vector<std::vector<float>> heights;
  std::vector<std::vector<glm::vec2>> gradients;
};
