#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "./OpenGLMesh.hpp"

namespace ShapeGenerator {
  std::unique_ptr<OpenGLMesh> generateCube(glm::vec3 min, glm::vec3 max, glm::vec4 color);
  std::unique_ptr<OpenGLMesh> generateCubeFromCenter(glm::vec3 center, glm::vec3 size, glm::vec4 color);

  std::unique_ptr<OpenGLMesh> generateSphere(glm::vec3 center, float radius, glm::vec4 color);
};


