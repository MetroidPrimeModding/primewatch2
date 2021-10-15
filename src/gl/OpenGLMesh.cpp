#include "OpenGLMesh.hpp"

#include <glad/glad.h>

OpenGLMesh::OpenGLMesh(const std::vector<Vert> &vertData, RenderType mode, BufferUpdateType type) {
  this->mode = mode;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*) offsetof(Vert, pos));
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*) offsetof(Vert, color));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*) offsetof(Vert, normal));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  bufferNewData(vertData, type);
}

OpenGLMesh::~OpenGLMesh() {
  if (this->vao != 0) {
    glDeleteVertexArrays(1, &this->vao);
  }
  if (this->vbo != 0) {
    glDeleteBuffers(1, &this->vbo);
  }
}

void OpenGLMesh::bufferNewData(const std::vector<Vert> &vertData, BufferUpdateType type) {
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  int bufferMode = GL_STATIC_DRAW;
  if (type == BufferUpdateType::STREAM) {
    bufferMode = GL_STREAM_DRAW;
  } else if (type == BufferUpdateType::DYNAMIC) {
    bufferMode = GL_DYNAMIC_DRAW;
  }
  vertCount = static_cast<int>(vertData.size());
  glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(Vert), vertData.data(), bufferMode);
}

void OpenGLMesh::draw() {
  glBindVertexArray(vao);

  int glMode = GL_POINTS;
  switch (mode) {
    case RenderType::POINTS:
      glMode = GL_POINTS;
      break;
    case RenderType::LINES:
      glMode = GL_LINES;
      break;
    case RenderType::LINE_LOOP:
      glMode = GL_LINE_LOOP;
      break;
    case RenderType::LINE_STRIP:
      glMode = GL_LINE_STRIP;
      break;
    case RenderType::TRIANGLES:
      glMode = GL_TRIANGLES;
      break;
    case RenderType::TRIANGLE_STRIP:
      glMode = GL_TRIANGLE_STRIP;
      break;
    case RenderType::TRIANGLE_FAN:
      glMode = GL_TRIANGLE_FAN;
      break;
  }
  glDrawArrays(glMode, 0, vertCount);
}

