#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "./OpenGLMesh.hpp"

namespace ShapeGenerator {
  std::vector<Vert> generateCube(glm::vec3 min, glm::vec3 max, glm::vec4 color);
  std::vector<Vert> generateCubeFromCenter(glm::vec3 center, glm::vec3 size, glm::vec4 color);
  std::vector<Vert> generateCubeLines(glm::vec3 min, glm::vec3 max, glm::vec4 color);
  std::vector<Vert> generateSphere(glm::vec3 center, float radius, glm::vec4 color);
  std::vector<Vert> generateTruncatedSphere(glm::vec3 center, float radius, float bottomDistance, glm::vec4 color);
  std::vector<Vert> generateCameraLineSegments(glm::mat4 perspective, glm::mat4 transform, float centerLineLength);
};


