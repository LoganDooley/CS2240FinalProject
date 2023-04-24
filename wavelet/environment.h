#pragma once

#include "glm/vec2.hpp"

class Environment {
public:
  bool inDomain(glm::vec2 pos) const;
  // should be distance from boundary
  float levelSet(glm::vec2 pos) const;
  // slope
  glm::vec2 levelSetGradient(glm::vec2 pos) const;
};
