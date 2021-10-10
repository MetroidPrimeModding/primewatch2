#include "OpenGLMesh.hpp"

#include <glad/glad.h>

OpenGLMesh::OpenGLMesh(std::vector<Tri> tris) {
  triCount = static_cast<int>(tris.size());

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, triCount * sizeof(Tri), tris.data(), GL_STATIC_DRAW);

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
  glDrawArrays(GL_TRIANGLES, 0, triCount * 3);
}
