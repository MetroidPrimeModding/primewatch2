#include "OpenGLMesh.hpp"

#include <glad/glad.h>

OpenGLMesh::OpenGLMesh(std::vector<Vert> vertData, RenderType mode) {
  vertCount = static_cast<int>(vertData.size());
  this->mode = mode;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(Vert), vertData.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*) offsetof(Vert, pos));
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*) offsetof(Vert, color));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*) offsetof(Vert, normal));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
}

OpenGLMesh::~OpenGLMesh() {
  if (this->vao != 0) {
    glDeleteVertexArrays(1, &this->vao);
  }
  if (this->vbo != 0) {
    glDeleteBuffers(1, &this->vbo);
  }
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
