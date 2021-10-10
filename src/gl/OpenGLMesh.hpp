#pragma once

#include <vector>
#include <glm/glm.hpp>

struct __attribute__ ((packed)) Vert {
  glm::vec3 pos;
  glm::vec4 color;
  glm::vec3 normal;
};

struct __attribute__ ((packed)) Tri {
  Vert a;
  Vert b;
  Vert c;
};

class OpenGLMesh {
public:
  explicit OpenGLMesh(std::vector<Tri> tris);
  ~OpenGLMesh();

  OpenGLMesh(const OpenGLMesh &) = delete;
  OpenGLMesh &operator=(const OpenGLMesh &) = delete;

  void draw();
private:
  int triCount{0};
  unsigned int vbo{0};
  unsigned int vao{0};
};


