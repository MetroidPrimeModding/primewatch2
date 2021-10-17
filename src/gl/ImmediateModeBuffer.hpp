#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "./OpenGLMesh.hpp"

class ImmediateModeBuffer {
public:
  explicit ImmediateModeBuffer();
  ~ImmediateModeBuffer() = default;

  ImmediateModeBuffer(const ImmediateModeBuffer &) = delete;
  ImmediateModeBuffer &operator=(const ImmediateModeBuffer &) = delete;

  void clear();
  void draw();

  void setColor(const glm::vec4 &color);
  void setTransform(const glm::mat4 &transform);
  void addLine(const glm::vec3 &start, const glm::vec3 &end);
  void addLine(const Vert &start, const Vert &end);
  void addTri(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
  void addTri(const Vert &a, const Vert &b, const Vert &c);
  void addQuad(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &d);
  void addQuad(const Vert &a, const Vert &b, const Vert &c, const Vert &d);

  void addLines(const std::vector<Vert> &newLineVerts);
  void addTris(const std::vector<Vert> &newTriVerts);

private:
  std::unique_ptr<OpenGLMesh> linesMesh{nullptr};
  std::unique_ptr<OpenGLMesh> triMesh{nullptr};
  glm::vec4 currentColor{1.0f, 1.0f, 1.0f, 1.0f};
  glm::mat4 vertTransform{1.0f};
  glm::mat3 normalTransform{1.0f};

  std::vector<Vert> lineVerts;
  std::vector<Vert> triVerts;
};


