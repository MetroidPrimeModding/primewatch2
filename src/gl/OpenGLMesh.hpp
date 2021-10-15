#pragma once

#include <vector>
#include <glm/glm.hpp>

#ifdef WIN32
#pragma pack(push,1)
struct
#else
struct __attribute__ ((packed)) 
#endif
	Vert {
  glm::vec3 pos;
  glm::vec4 color;
  glm::vec3 normal;
};
#ifdef WIN32
#pragma pack(pop)
#endif

enum class RenderType {
  POINTS,
  LINES,
  LINE_LOOP,
  LINE_STRIP,
  TRIANGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
};

enum class BufferUpdateType {
  STATIC,
  DYNAMIC,
  STREAM
};

class OpenGLMesh {
public:
  explicit OpenGLMesh(const std::vector<Vert> &vertData, RenderType mode, BufferUpdateType type = BufferUpdateType::STATIC);
  ~OpenGLMesh();

  OpenGLMesh(const OpenGLMesh &) = delete;
  OpenGLMesh &operator=(const OpenGLMesh &) = delete;

  void bufferNewData(const std::vector<Vert> &vertData, BufferUpdateType type = BufferUpdateType::DYNAMIC);
  void draw();
private:
  int vertCount{0};
  RenderType mode{};
  unsigned int vbo{0};
  unsigned int vao{0};
};


