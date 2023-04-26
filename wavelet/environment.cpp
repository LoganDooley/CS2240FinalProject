#include "environment.h"
#include <cfloat>

Environment::Environment(std::string filename, float zBoundary) {
  int width, height, n;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &n, 0);
  if (data == NULL) {
    std::cout << "Error loading image" << std::endl;
    exit(1);
  }
  heights.resize(width);
  gradients.resize(width);
  for (int i = 0; i < width; i++) {
    heights[i].resize(height);
    gradients[i].resize(height);
    for (int j = 0; j < height; j++) {
      int index = (i + j * width) * n;
      float r = data[index] / 255.0f;
      float g = data[index + 1] / 255.0f;
      float b = data[index + 2] / 255.0f;
      float height = (r + g + b) / 3.0f;
      heights[i][j] = height;
      gradients[i][j] = glm::vec2(0, 0);
    }
  }
  for (int i = 1; i < width - 1; i++) {
    for (int j = 1; j < height - 1; j++) {
      float dx = heights[i + 1][j] - heights[i - 1][j];
      float dy = heights[i][j + 1] - heights[i][j - 1];
      gradients[i][j] = glm::normalize(glm::vec2(dx, dy));
    }
  }
  stbi_image_free(data);
  this->zBoundary = zBoundary;
}

bool Environment::inDomain(glm::vec2 pos) const { return levelSet(pos) >= 0; }

float Environment::levelSet(glm::vec2 pos) const {
    if (pos.x < 0 || pos.x >= heights.size() || pos.y < 0 || pos.y >= heights[0].size())
        return -1;
  float zVal = heights[(int)((pos.x))][(int)((pos.y))];
  if (zVal < zBoundary) {
    return zBoundary - zVal;
  } else {
    return -1;
  }
}

glm::vec2 Environment::levelSetGradient(glm::vec2 pos) const {
  return gradients[(int)((pos.x))][(int)((pos.y))];
}
