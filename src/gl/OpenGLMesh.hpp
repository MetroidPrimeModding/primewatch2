#pragma once

#include <vector>
#include <glm/glm.hpp>

struct __attribute__ ((packed)) Vert {
  glm::vec3 pos;
  glm::vec4 color;
  glm::vec3 normal;
};

enum class RenderType {
  POINTS,
  LINES,
  LINE_LOOP,
  LINE_STRIP,
  TRIANGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
};

class OpenGLMesh {
public:
  explicit OpenGLMesh(std::vector<Vert> vertData, RenderType mode);
  ~OpenGLMesh();

  OpenGLMesh(const OpenGLMesh &) = delete;
  OpenGLMesh &operator=(const OpenGLMesh &) = delete;

  void draw();
private:
  int vertCount{0};
  RenderType mode{};
  unsigned int vbo{0};
  unsigned int vao{0};
};


