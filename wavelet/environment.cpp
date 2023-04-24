#include "wavelet/environment.h"
#include <cfloat>

bool Environment::inDomain(glm::vec2 pos) const { return levelSet(pos) >= 0; }

// make a box with flat sides
float Environment::levelSet(glm::vec2 pos) const {
  if (pos.x < 1 && pos.x > -1 && pos.y < 1 && pos.y > -1) {
    return 1;
  } else if (pos.x == 1 || pos.x == -1 || pos.y == 1 || pos.y == -1) {
    return 0;
  } else {
    return -1;
  }
}

glm::vec2 Environment::levelSetGradient(glm::vec2 pos) const {
  if (pos.x < 1 && pos.x > -1 && pos.y < 1 && pos.y > -1) {
    return glm::vec2(0, 0);
  } else if (pos.x == 1 || pos.x == -1 || pos.y == 1 || pos.y == -1) {
    return glm::vec2(FLT_MAX, FLT_MAX);
  } else {
    return glm::vec2(-1, -1);
  }
}
