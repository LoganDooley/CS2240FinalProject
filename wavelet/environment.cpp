#include "wavelet/environment.h"

bool Environment::inDomain(glm::vec2 pos) const {
    return levelSet(pos) >= 0;
}

float Environment::levelSet(glm::vec2 pos) const {
    return 0;
}

glm::vec2 Environment::levelSetGradient(glm::vec2 pos) const {
    return glm::vec2(0,0);
}
